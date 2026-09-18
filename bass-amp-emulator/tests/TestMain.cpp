#include "TestHarness.h"

void testFft();
void testConvolver();
void testOversampler();
void testIrDesigner();
void testTuner();
void testMetronome();
void testAmpAndCab();
void testPedals();

int main()
{
    std::printf ("BassAmp DSP core test suite\n===========================\n");
    testFft();
    testConvolver();
    testOversampler();
    testIrDesigner();
    testTuner();
    testMetronome();
    testAmpAndCab();
    testPedals();
    return tst::report();
}
