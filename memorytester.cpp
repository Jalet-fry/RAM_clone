#include "memorytester.h"
#include "constants.h"
#include <QThread>
#include <cmath>

MemoryTester::MemoryTester(MemoryModel* mem, QObject* parent)
    : QObject(parent), _mem(mem) {}

void MemoryTester::writePattern(size_t addr, Word pattern) {
    _mem->writeDirect(addr, pattern);
}

void MemoryTester::readAndVerify(size_t addr, Word expected) {
    Word read = _mem->read(addr);
    bool pass = (expected == read);
    _results.push_back({addr, expected, read, pass});
    // Get size safely (thread-safe method)
    size_t memSize = _mem->size();
    if (addr % PROGRESS_UPDATE_INTERVAL == 0 || (memSize > 0 && addr == memSize - 1)) {
        emit progressDetail(addr, expected, read);
    }
}

void MemoryTester::updateProgress(size_t current, size_t total, double phasePercent, double basePercent) {
    if (current % PROGRESS_UPDATE_INTERVAL == 0 || current == total - 1) {
        int percent = int(basePercent) + int((current * phasePercent) / total);
        emit progress(percent);
        QThread::msleep(VISUALIZATION_DELAY_MS);
    }
}

void MemoryTester::runTest(TestAlgorithm algo) {
    _results.clear();
    size_t n = _mem->size(); // Thread-safe call
    if (n == 0) { 
        emit progress(PROGRESS_MAX_PERCENT); 
        emit finished(_results); 
        return; 
    }

    if (algo == TestAlgorithm::WalkingOnes) {
        // Phase 1: Write reference data
        for (size_t a = 0; a < n; ++a) {
            Word pattern = (1u << (a % BITS_PER_WORD));
            writePattern(a, pattern);
            updateProgress(a, n, PROGRESS_PHASE_PERCENT);
        }
        
        // Phase 2: Read and verify (faults are applied here)
        for (size_t a = 0; a < n; ++a) {
            Word expected = (1u << (a % BITS_PER_WORD)); // What we wrote
            readAndVerify(a, expected);
            updateProgress(a, n, PROGRESS_PHASE_PERCENT, PROGRESS_PHASE_PERCENT);
        }
        emit progress(PROGRESS_MAX_PERCENT); // Guarantee 100% at the end
        
    } else if (algo == TestAlgorithm::WalkingZeros) {
        // Phase 1: Write reference data
        for (size_t a = 0; a < n; ++a) {
            Word pattern = ~(1u << (a % BITS_PER_WORD));
            writePattern(a, pattern);
            updateProgress(a, n, PROGRESS_PHASE_PERCENT);
        }
        
        // Phase 2: Read and verify
        for (size_t a = 0; a < n; ++a) {
            Word expected = ~(1u << (a % BITS_PER_WORD));
            readAndVerify(a, expected);
            updateProgress(a, n, PROGRESS_PHASE_PERCENT, PROGRESS_PHASE_PERCENT);
        }
        emit progress(PROGRESS_MAX_PERCENT); // Guarantee 100% at the end
        
    } else if (algo == TestAlgorithm::MarchSimple) {
        // March test: write 0 → read 0 → write 1 → read 1
        
        // Step 1: Write all 0s
        for (size_t a = 0; a < n; ++a) {
            writePattern(a, 0u);
            updateProgress(a, n, PROGRESS_MARCH_PERCENT);
        }
        
        // Step 2: Read all 0s (expect 0)
        for (size_t a = 0; a < n; ++a) {
            readAndVerify(a, 0u);
            updateProgress(a, n, PROGRESS_MARCH_PERCENT, PROGRESS_MARCH_PERCENT);
        }
        
        // Step 3: Write all 1s
        for (size_t a = 0; a < n; ++a) {
            writePattern(a, ~0u);
            updateProgress(a, n, PROGRESS_MARCH_PERCENT, PROGRESS_MARCH_PERCENT * 2);
        }
        
        // Step 4: Read all 1s (expect 1)
        for (size_t a = 0; a < n; ++a) {
            readAndVerify(a, ~0u);
            updateProgress(a, n, PROGRESS_MARCH_PERCENT, PROGRESS_MARCH_PERCENT * 3);
        }
        emit progress(PROGRESS_MAX_PERCENT); // Guarantee 100% at the end
    }

    emit finished(_results);
}