# Run this script to convert the edge-impulse-sdk folder into a library that can be consumed by the Arduino IDE
# it renames files (e.g. *.cpp to *.c), removes features (uTensor), and updates include paths

# exit when any command fails
set -e

cleanup() {
    echo ""
    echo "Terminated by user"
    exit 1
}
trap cleanup INT TERM

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

rm -rf $SCRIPTPATH/utensor
rm -rf $SCRIPTPATH/tensorflow/lite/micro/mbed/
rm -rf $SCRIPTPATH/porting/posix/
rm -rf $SCRIPTPATH/porting/stm32-cubeai/
rm -rf $SCRIPTPATH/classifier/ei_run_classifier_c*
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal2.S
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/MatrixFunctions/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/TransformFunctions/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/CommonTables/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/FilteringFunctions/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/StatisticsFunctions/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/BasicMathFunctions/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/SupportFunctions/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/DistanceFunctions/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/ControllerFunctions/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/FastMathFunctions/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/BayesFunctions/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/SVMFunctions/*.c
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/ComplexMathFunctions/*.c

# rename all .cc files to .cpp, and do an inplace change of the headers
find . -name '*.cc' -exec sh -c 'mv "$0" "${0%.cc}.cpp"' {} \;

# fix headers in tensorflow directory
find $SCRIPTPATH/ -name '*.h' -exec bash -c "sed -i -e 's/#include \"tensorflow\//#include \"edge-impulse-sdk\/tensorflow\//' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "sed -i -e 's/#include \"tensorflow\//#include \"edge-impulse-sdk\/tensorflow\//' {}" {} \;
find $SCRIPTPATH/ -name '*.h' -exec bash -c "sed -i -e 's/#include \"flatbuffers\//#include \"edge-impulse-sdk\/third_party\/flatbuffers\/include\/flatbuffers\//' {}" {} \;

# fix headers for flatbuffers
find $SCRIPTPATH/third_party/flatbuffers -name '*.h' -exec bash -c "sed -i -e 's/#include \"flatbuffers\//#include \"edge-impulse-sdk\/third_party\/flatbuffers\/include\/flatbuffers\//' {}" {} \;
find $SCRIPTPATH/third_party/flatbuffers -name '*.h' -exec bash -c "sed -i -e 's/#include <utility.h>/#include <utility>/' {}" {} \;

# CMSIS-NN headers
find $SCRIPTPATH/ -name '*.h' -exec bash -c "sed -i -e 's/#include \"arm_nnfunctions/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nnfunctions/' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "sed -i -e 's/#include \"arm_nnfunctions/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nnfunctions/' {}" {} \;
find $SCRIPTPATH/ -name '*.h' -exec bash -c "sed -i -e 's/#include \"arm_nnsupportfunctions/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nnsupportfunctions/' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "sed -i -e 's/#include \"arm_nnsupportfunctions/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nnsupportfunctions/' {}" {} \;
find $SCRIPTPATH/ -name '*.h' -exec bash -c "sed -i -e 's/#include \"arm_nntables/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nntables/' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "sed -i -e 's/#include \"arm_nntables/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nntables/' {}" {} \;
find $SCRIPTPATH/ -name '*.h' -exec bash -c "sed -i -e 's/#include \"arm_common_tables/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_common_tables/' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "sed -i -e 's/#include \"arm_common_tables/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_common_tables/' {}" {} \;
find $SCRIPTPATH/ -name '*.h' -exec bash -c "sed -i -e 's/#include \"arm_math/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_math/' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "sed -i -e 's/#include \"arm_math/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_math/' {}" {} \;
find $SCRIPTPATH/ -name '*.h' -exec bash -c "sed -i -e 's/#include \"cmsis_compiler/#include \"edge-impulse-sdk\/CMSIS\/Core\/Include\/cmsis_compiler/' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "sed -i -e 's/#include \"cmsis_compiler/#include \"edge-impulse-sdk\/CMSIS\/Core\/Include\/cmsis_compiler/' {}" {} \;
find $SCRIPTPATH/ -name '*.h' -exec bash -c "sed -i -e 's/#include \"ruy\/profiler\/instrumentation/#include \"edge-impulse-sdk\/third_party\/ruy\/ruy\/profiler\/instrumentation/' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "sed -i -e 's/#include \"ruy\/profiler\/instrumentation/#include \"edge-impulse-sdk\/third_party\/ruy\/ruy\/profiler\/instrumentation/' {}" {} \;
find $SCRIPTPATH/ -name '*.h' -exec bash -c "sed -i -e 's/#include \"fixedpoint\/fixedpoint/#include \"edge-impulse-sdk\/third_party\/gemmlowp\/fixedpoint\/fixedpoint/' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "sed -i -e 's/#include \"fixedpoint\/fixedpoint/#include \"edge-impulse-sdk\/third_party\/gemmlowp\/fixedpoint\/fixedpoint/' {}" {} \;

# remove all the -e files
find $SCRIPTPATH/ -name "*-e" -exec rm -f {} \;
