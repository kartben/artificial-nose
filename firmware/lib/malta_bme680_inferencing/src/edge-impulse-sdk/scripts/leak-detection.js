// How to use this script:
// 1. Enable the 'EIDSP_TRACK_ALLOCATIONS' macro in dsp/config.hpp
// 2. Run the application, and paste the full output below (in the report variable)
// 3. Run the script via:
//    $ node leak-detection.js
// 4. You'll see exactly if there's any leaks

const report = `alloc matrix 2 1 x 21 = 84 bytes (in_use=84, peak=84) (ei_matrix@edge-impulse-sdk/dsp/numpy_types.h:117) 0x7fd8bc4058a0
alloc matrix 2 200 x 3 = 2400 bytes (in_use=2484, peak=2484) (ei_matrix@edge-impulse-sdk/dsp/numpy_types.h:117) 0x7fd8bc80da00
alloc matrix 2 3 x 200 = 2400 bytes (in_use=4884, peak=4884) (transpose@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:324) 0x7fd8bc80e400
free matrix 3 x 200 = 2400 bytes (in_use=2484, peak=4884) (transpose@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:324) 0x7fd8bc80e400
alloc matrix 2 64 x 1 = 256 bytes (in_use=2740, peak=4884) (ei_matrix@edge-impulse-sdk/dsp/numpy_types.h:117) 0x7fd8bc4059a0
alloc matrix 2 3 x 1 = 12 bytes (in_use=2752, peak=4884) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:85) 0x7fd8bc405900
alloc matrix 2 3 x 1 = 12 bytes (in_use=2764, peak=4884) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:114) 0x7fd8bc405960
alloc matrix 2 3 x 2 = 24 bytes (in_use=2788, peak=4884) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:121) 0x7fd8bc405970
alloc matrix 2 1 x 65 = 260 bytes (in_use=3048, peak=4884) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:130) 0x7fd8bc405aa0
alloc matrix 2 1 x 128 = 512 bytes (in_use=3560, peak=4884) (rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:1316) 0x7fd8bc405ca0
alloc 520 bytes (in_use=4080, peak=4884) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2045) 0x7fd8bc405ea0
alloc 1568 bytes (in_use=5648, peak=5648) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2059) 0x7fd8bd008200
free 1568 bytes (in_use=4080, peak=5648) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2069) 0x7fd8bd008200
free 520 bytes (in_use=3560, peak=5648) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2070) 0x7fd8bc405ea0
free matrix 1 x 128 = 512 bytes (in_use=3048, peak=5648) (rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:1316) 0x7fd8bc405ca0
alloc matrix 2 1 x 2 = 8 bytes (in_use=3056, peak=5648) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:140) 0x7fd8bc405990
alloc matrix 2 1 x 65 = 260 bytes (in_use=3316, peak=5648) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:380) 0x7fd8bc405bb0
alloc matrix 2 10 x 1 = 40 bytes (in_use=3356, peak=5648) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:386) 0x7fd8bc405910
free matrix 10 x 1 = 40 bytes (in_use=3316, peak=5648) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:386) 0x7fd8bc405910
free matrix 1 x 65 = 260 bytes (in_use=3056, peak=5648) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:380) 0x7fd8bc405bb0
alloc matrix 2 1 x 65 = 260 bytes (in_use=3316, peak=5648) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:148) 0x7fd8bc405bb0
alloc matrix 2 1 x 65 = 260 bytes (in_use=3576, peak=5648) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:149) 0x7fd8bc405cc0
alloc matrix 2 1 x 128 = 512 bytes (in_use=4088, peak=5648) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:766) 0x7fd8bc405dd0
alloc matrix 2 1 x 1 = 4 bytes (in_use=4092, peak=5648) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:780) 0x7fd8bc405940
alloc 520 bytes (in_use=4612, peak=5648) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:791) 0x7fd8bc4060b0
alloc 1568 bytes (in_use=6180, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2085) 0x7fd8bc80ae00
free 1568 bytes (in_use=4612, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2090) 0x7fd8bc80ae00
free 520 bytes (in_use=4092, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:814) 0x7fd8bc4060b0
free matrix 1 x 1 = 4 bytes (in_use=4088, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:780) 0x7fd8bc405940
free matrix 1 x 128 = 512 bytes (in_use=3576, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:766) 0x7fd8bc405dd0
alloc matrix 2 4 x 1 = 16 bytes (in_use=3592, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:156) 0x7fd8bc405940
alloc matrix 2 1 x 4 = 16 bytes (in_use=3608, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:625) 0x7fd8bc405950
alloc matrix 2 1 x 4 = 16 bytes (in_use=3624, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:626) 0x7fd8bc504080
free matrix 1 x 4 = 16 bytes (in_use=3608, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:626) 0x7fd8bc504080
free matrix 1 x 4 = 16 bytes (in_use=3592, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:625) 0x7fd8bc405950
free matrix 4 x 1 = 16 bytes (in_use=3576, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:156) 0x7fd8bc405940
free matrix 1 x 65 = 260 bytes (in_use=3316, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:149) 0x7fd8bc405cc0
free matrix 1 x 65 = 260 bytes (in_use=3056, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:148) 0x7fd8bc405bb0
free matrix 1 x 2 = 8 bytes (in_use=3048, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:140) 0x7fd8bc405990
free matrix 1 x 65 = 260 bytes (in_use=2788, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:130) 0x7fd8bc405aa0
alloc matrix 2 1 x 65 = 260 bytes (in_use=3048, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:130) 0x7fd8bc604080
alloc matrix 2 1 x 128 = 512 bytes (in_use=3560, peak=6180) (rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:1316) 0x7fd8bc604190
alloc 520 bytes (in_use=4080, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2045) 0x7fd8bc604390
alloc 1568 bytes (in_use=5648, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2059) 0x7fd8bd008200
free 1568 bytes (in_use=4080, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2069) 0x7fd8bd008200
free 520 bytes (in_use=3560, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2070) 0x7fd8bc604390
free matrix 1 x 128 = 512 bytes (in_use=3048, peak=6180) (rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:1316) 0x7fd8bc604190
alloc matrix 2 1 x 2 = 8 bytes (in_use=3056, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:140) 0x7fd8bc604190
alloc matrix 2 1 x 65 = 260 bytes (in_use=3316, peak=6180) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:380) 0x7fd8bc6041a0
alloc matrix 2 10 x 1 = 40 bytes (in_use=3356, peak=6180) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:386) 0x7fd8bc6042b0
free matrix 10 x 1 = 40 bytes (in_use=3316, peak=6180) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:386) 0x7fd8bc6042b0
free matrix 1 x 65 = 260 bytes (in_use=3056, peak=6180) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:380) 0x7fd8bc6041a0
alloc matrix 2 1 x 65 = 260 bytes (in_use=3316, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:148) 0x7fd8bc704080
alloc matrix 2 1 x 65 = 260 bytes (in_use=3576, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:149) 0x7fd8bc704190
alloc matrix 2 1 x 128 = 512 bytes (in_use=4088, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:766) 0x7fd8bc7042a0
alloc matrix 2 1 x 1 = 4 bytes (in_use=4092, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:780) 0x7fd8bc7044a0
alloc 520 bytes (in_use=4612, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:791) 0x7fd8bc7044b0
alloc 1568 bytes (in_use=6180, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2085) 0x7fd8bd808200
free 1568 bytes (in_use=4612, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2090) 0x7fd8bd808200
free 520 bytes (in_use=4092, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:814) 0x7fd8bc7044b0
free matrix 1 x 1 = 4 bytes (in_use=4088, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:780) 0x7fd8bc7044a0
free matrix 1 x 128 = 512 bytes (in_use=3576, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:766) 0x7fd8bc7042a0
alloc matrix 2 4 x 1 = 16 bytes (in_use=3592, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:156) 0x7fd8bc7044a0
alloc matrix 2 1 x 4 = 16 bytes (in_use=3608, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:625) 0x7fd8bc7042a0
alloc matrix 2 1 x 4 = 16 bytes (in_use=3624, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:626) 0x7fd8bc7042b0
free matrix 1 x 4 = 16 bytes (in_use=3608, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:626) 0x7fd8bc7042b0
free matrix 1 x 4 = 16 bytes (in_use=3592, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:625) 0x7fd8bc7042a0
free matrix 4 x 1 = 16 bytes (in_use=3576, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:156) 0x7fd8bc7044a0
free matrix 1 x 65 = 260 bytes (in_use=3316, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:149) 0x7fd8bc704190
free matrix 1 x 65 = 260 bytes (in_use=3056, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:148) 0x7fd8bc704080
free matrix 1 x 2 = 8 bytes (in_use=3048, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:140) 0x7fd8bc604190
free matrix 1 x 65 = 260 bytes (in_use=2788, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:130) 0x7fd8bc604080
alloc matrix 2 1 x 65 = 260 bytes (in_use=3048, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:130) 0x7fd8be004080
alloc matrix 2 1 x 128 = 512 bytes (in_use=3560, peak=6180) (rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:1316) 0x7fd8be004190
alloc 520 bytes (in_use=4080, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2045) 0x7fd8be004390
alloc 1568 bytes (in_use=5648, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2059) 0x7fd8be808200
free 1568 bytes (in_use=4080, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2069) 0x7fd8be808200
free 520 bytes (in_use=3560, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2070) 0x7fd8be004390
free matrix 1 x 128 = 512 bytes (in_use=3048, peak=6180) (rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:1316) 0x7fd8be004190
alloc matrix 2 1 x 2 = 8 bytes (in_use=3056, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:140) 0x7fd8be004190
alloc matrix 2 1 x 65 = 260 bytes (in_use=3316, peak=6180) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:380) 0x7fd8be0041a0
alloc matrix 2 10 x 1 = 40 bytes (in_use=3356, peak=6180) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:386) 0x7fd8be0042b0
free matrix 10 x 1 = 40 bytes (in_use=3316, peak=6180) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:386) 0x7fd8be0042b0
free matrix 1 x 65 = 260 bytes (in_use=3056, peak=6180) (find_fft_peaks@./edge-impulse-sdk/dsp/spectral/processing.hpp:380) 0x7fd8be0041a0
alloc matrix 2 1 x 65 = 260 bytes (in_use=3316, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:148) 0x7fd8be0041a0
alloc matrix 2 1 x 65 = 260 bytes (in_use=3576, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:149) 0x7fd8be0042e0
alloc matrix 2 1 x 128 = 512 bytes (in_use=4088, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:766) 0x7fd8be0045a0
alloc matrix 2 1 x 1 = 4 bytes (in_use=4092, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:780) 0x7fd8be0043f0
alloc 520 bytes (in_use=4612, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:791) 0x7fd8be0047a0
alloc 1568 bytes (in_use=6180, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2085) 0x7fd8be808200
free 1568 bytes (in_use=4612, peak=6180) (software_rfft@./edge-impulse-sdk/dsp/spectral/../numpy.hpp:2090) 0x7fd8be808200
free 520 bytes (in_use=4092, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:814) 0x7fd8be0047a0
free matrix 1 x 1 = 4 bytes (in_use=4088, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:780) 0x7fd8be0043f0
free matrix 1 x 128 = 512 bytes (in_use=3576, peak=6180) (periodogram@./edge-impulse-sdk/dsp/spectral/processing.hpp:766) 0x7fd8be0045a0
alloc matrix 2 4 x 1 = 16 bytes (in_use=3592, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:156) 0x7fd8be0043f0
alloc matrix 2 1 x 4 = 16 bytes (in_use=3608, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:625) 0x7fd8be0042b0
alloc matrix 2 1 x 4 = 16 bytes (in_use=3624, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:626) 0x7fd8be0042c0
free matrix 1 x 4 = 16 bytes (in_use=3608, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:626) 0x7fd8be0042c0
free matrix 1 x 4 = 16 bytes (in_use=3592, peak=6180) (spectral_power_edges@./edge-impulse-sdk/dsp/spectral/processing.hpp:625) 0x7fd8be0042b0
free matrix 4 x 1 = 16 bytes (in_use=3576, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:156) 0x7fd8be0043f0
free matrix 1 x 65 = 260 bytes (in_use=3316, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:149) 0x7fd8be0042e0
free matrix 1 x 65 = 260 bytes (in_use=3056, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:148) 0x7fd8be0041a0
free matrix 1 x 2 = 8 bytes (in_use=3048, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:140) 0x7fd8be004190
free matrix 1 x 65 = 260 bytes (in_use=2788, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:130) 0x7fd8be004080
free matrix 3 x 2 = 24 bytes (in_use=2764, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:121) 0x7fd8bc405970
free matrix 3 x 1 = 12 bytes (in_use=2752, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:114) 0x7fd8bc405960
free matrix 3 x 1 = 12 bytes (in_use=2740, peak=6180) (spectral_analysis@./edge-impulse-sdk/dsp/spectral/feature.hpp:85) 0x7fd8bc405900
free matrix 5 x 1 = 20 bytes (in_use=2720, peak=6180) (~ei_matrix@edge-impulse-sdk/dsp/numpy_types.h:132) 0x7fd8bc4059a0
free matrix 3 x 200 = 2400 bytes (in_use=320, peak=6180) (~ei_matrix@edge-impulse-sdk/dsp/numpy_types.h:132) 0x7fd8bc80da00
Features (2 ms.): 0.190703 2.380952 0.147790 0.000000 0.000872 0.000080 0.000395 1.033266 0.793651 1.161580 0.000000 0.086353 0.034655 0.002571 0.510732 0.793651 0.648773 0.000000 0.026938 0.004146 0.001570
Running neural network...
Predictions (time: 0 ms.):
updown: 0.996094
free matrix 1 x 21 = 84 bytes (in_use=236, peak=6180) (~ei_matrix@edge-impulse-sdk/dsp/numpy_types.h:132) 0x7fd8bc4058a0`;

let allocated = [];

for (let line of report.split('\n')) {
    let s = line.split(' ');
    let ptr = s[s.length - 1];

    if (line.startsWith('alloc')) {
        let splitted = line.split(' bytes')[0].split(' ');
        let alloc = Number(splitted[splitted.length - 1]);
        // console.log('alloc', alloc);
        allocated.push({
            ptr: ptr,
            size: alloc
        });
    }
    else if (line.startsWith('free')) {
        let splitted = line.split(' bytes')[0].split(' ');
        let free = Number(splitted[splitted.length - 1]);

        let p = allocated.find(x => x.ptr === ptr && x.size === free);
        if (!p) {
            console.warn('Could not find ptr', ptr, free);
        }
        else {
            allocated.splice(allocated.indexOf(p), 1);
        }
    }
}

console.log('dangling', allocated);
