#include "TestHarness.h"

void testFft();
void testConvolver();
void testOversampler();
void testIrDesigner();
void testTuner();
void testMetronome();
void testAmpAndCab();
void testPedals();
void testRig();
void testPresets();
void testStateRoundTrip();

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
    testRig();
    testPresets();
    testStateRoundTrip();
    return tst::report();
}
