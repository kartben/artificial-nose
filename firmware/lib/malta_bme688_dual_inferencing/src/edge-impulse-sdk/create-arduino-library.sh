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
rm -rf $SCRIPTPATH/porting/mbed/
rm -rf $SCRIPTPATH/porting/mingw32/
rm -rf $SCRIPTPATH/porting/posix/
rm -rf $SCRIPTPATH/porting/silabs/
rm -rf $SCRIPTPATH/porting/stm32-cubeai/
rm -rf $SCRIPTPATH/porting/zephyr/
rm -rf $SCRIPTPATH/porting/sony/
rm -rf $SCRIPTPATH/porting/ti/
rm -rf $SCRIPTPATH/porting/lib/
rm -rf $SCRIPTPATH/porting/raspberry/
rm -rf $SCRIPTPATH/porting/himax/
rm -rf $SCRIPTPATH/porting/synaptics/
rm -rf $SCRIPTPATH/classifier/ei_run_classifier_c*
rm -rf $SCRIPTPATH/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal2.S
rm -rf $SCRIPTPATH/third_party/arc_mli_package/

# rename all .cc files to .cpp, and do an inplace change of the headers
find . -name '*.cc' -exec sh -c 'mv "$0" "${0%.cc}.cpp"' {} \;

# make sure that abs is undefined on arduino
find $SCRIPTPATH/ -name 'compatibility.h' -exec bash -c "$SEDCMD 's/#include <cstdint>/#include <cstdint>\\
#include \"edge-impulse-sdk\/tensorflow\/lite\/portable_type_to_tflitetype.h\"/' {}" {} \;
find $SCRIPTPATH/ -name 'micro_utils.h' -exec bash -c "$SEDCMD 's/#include <cstdint>/#include <cstdint>\\
#include \"edge-impulse-sdk\/tensorflow\/lite\/portable_type_to_tflitetype.h\"/' {}" {} \;

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
