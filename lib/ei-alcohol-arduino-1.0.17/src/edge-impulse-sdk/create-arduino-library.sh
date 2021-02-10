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

if [[ "$OSTYPE" == "darwin"* ]]; then
    SEDCMD="sed -i '' -e"
    ECHOCMD="echo"
    LC_CTYPE=C
    LANG=C
else
    SEDCMD="sed -i -e"
    ECHOCMD="echo -e"
fi

rm -rf $SCRIPTPATH/tensorflow/lite/micro/mbed/
rm -rf $SCRIPTPATH/porting/ecm3532/
rm -rf $SCRIPTPATH/porting/himax/
rm -rf $SCRIPTPATH/porting/mbed/
rm -rf $SCRIPTPATH/porting/mingw32/
rm -rf $SCRIPTPATH/porting/posix/
rm -rf $SCRIPTPATH/porting/silabs/
rm -rf $SCRIPTPATH/porting/stm32-cubeai/
rm -rf $SCRIPTPATH/porting/zephyr/
rm -rf $SCRIPTPATH/classifier/ei_run_classifier_c*
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal2.S
rm -rf $SCRIPTPATH/third_party/arc_mli_package/

# rename all .cc files to .cpp, and do an inplace change of the headers
find . -name '*.cc' -exec sh -c 'mv "$0" "${0%.cc}.cpp"' {} \;

# fix headers in tensorflow directory
find $SCRIPTPATH/ -name '*.h' -exec bash -c "$SEDCMD 's/#include \"tensorflow\//#include \"edge-impulse-sdk\/tensorflow\//' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"tensorflow\//#include \"edge-impulse-sdk\/tensorflow\//' {}" {} \;
find $SCRIPTPATH/ -name '*.h' -exec bash -c "$SEDCMD 's/#include \"flatbuffers\//#include \"edge-impulse-sdk\/third_party\/flatbuffers\/include\/flatbuffers\//' {}" {} \;

# fix headers for flatbuffers
find $SCRIPTPATH/third_party/flatbuffers -name '*.h' -exec bash -c "$SEDCMD 's/#include \"flatbuffers\//#include \"edge-impulse-sdk\/third_party\/flatbuffers\/include\/flatbuffers\//' {}" {} \;
find $SCRIPTPATH/third_party/flatbuffers -name '*.h' -exec bash -c "$SEDCMD 's/#include <utility.h>/#include <utility>/' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"flatbuffers\//#include \"edge-impulse-sdk\/third_party\/flatbuffers\/include\/flatbuffers\//' {}" {} \;

# CMSIS-NN headers
find $SCRIPTPATH/CMSIS/ -name '*.h' -exec bash -c "$SEDCMD 's/#include \"arm_nnfunctions/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nnfunctions/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_nnfunctions/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nnfunctions/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.h' -exec bash -c "$SEDCMD 's/#include \"arm_nnsupportfunctions/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nnsupportfunctions/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_nnsupportfunctions/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nnsupportfunctions/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.h' -exec bash -c "$SEDCMD 's/#include \"arm_nntables/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nntables/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_nntables/#include \"edge-impulse-sdk\/CMSIS\/NN\/Include\/arm_nntables/' {}" {} \;

find $SCRIPTPATH/CMSIS/ -name '*.h' -exec bash -c "$SEDCMD 's/#include \"arm_common_tables.h/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_common_tables.h/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_common_tables.h/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_common_tables.h/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_common_tables_f16.h/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_common_tables_f16.h/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.h' -exec bash -c "$SEDCMD 's/#include \"arm_math/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_math/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_math/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_math/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_helium/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_helium/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_vec_/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_vec_/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_mve_tables.h/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_mve_tables.h/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_mve_tables_f16.h/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_mve_tables_f16.h/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_sorting/#include \"edge-impulse-sdk\/CMSIS\/DSP\/PrivateInclude\/arm_sorting/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_const_structs.h/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_const_structs.h/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_const_structs_f16.h/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Include\/arm_const_structs_f16.h/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.h' -exec bash -c "$SEDCMD 's/#include \"cmsis_compiler/#include \"edge-impulse-sdk\/CMSIS\/Core\/Include\/cmsis_compiler/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"cmsis_compiler/#include \"edge-impulse-sdk\/CMSIS\/Core\/Include\/cmsis_compiler/' {}" {} \;
find $SCRIPTPATH/CMSIS/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"arm_boolean_distance/#include \"edge-impulse-sdk\/CMSIS\/DSP\/Source\/DistanceFunctions\/arm_boolean_distance/' {}" {} \;

find $SCRIPTPATH/ -name '*.h' -exec bash -c "$SEDCMD 's/#include \"ruy\/profiler\/instrumentation/#include \"edge-impulse-sdk\/third_party\/ruy\/ruy\/profiler\/instrumentation/' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"ruy\/profiler\/instrumentation/#include \"edge-impulse-sdk\/third_party\/ruy\/ruy\/profiler\/instrumentation/' {}" {} \;
find $SCRIPTPATH/ -name '*.h' -exec bash -c "$SEDCMD 's/#include \"fixedpoint\/fixedpoint/#include \"edge-impulse-sdk\/third_party\/gemmlowp\/fixedpoint\/fixedpoint/' {}" {} \;
find $SCRIPTPATH/ -name '*.c*' -exec bash -c "$SEDCMD 's/#include \"fixedpoint\/fixedpoint/#include \"edge-impulse-sdk\/third_party\/gemmlowp\/fixedpoint\/fixedpoint/' {}" {} \;

# make sure that abs is undefined on arduino
find $SCRIPTPATH/ -name 'compatibility.h' -exec bash -c "$SEDCMD 's/#include <cstdint>/#include <cstdint>\\
#include \"edge-impulse-sdk\/tensorflow\/lite\/type_to_tflitetype.h\"/' {}" {} \;

# wrap all CMSIS-DSP .c files in a guard (defined in config.hpp)
find $SCRIPTPATH/CMSIS/DSP/Source -name "*.c" -print0 | while read -d $'\0' file; do
    $SEDCMD '1i\
#include \"edge-impulse-sdk/dsp/config.hpp\"\
#if EIDSP_LOAD_CMSIS_DSP_SOURCES
' "$file"

    $ECHOCMD '\n#endif // EIDSP_LOAD_CMSIS_DSP_SOURCES' >> "$file"
done

# remove all the -e files
find $SCRIPTPATH/ -name "*-e" -exec rm -f {} \;
