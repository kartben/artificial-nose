#include <Arduino.h>
#include "CliMode.h"
#include "Storage.h"
#include "Signature.h"

#define END_CHAR        ('\r')
#define TAB_CHAR        ('\t')
#define SPACE_CHAR      (' ')
#define BACKSPACE_CHAR  (0x08)
#define DEL_CHAR        (0x7f)

#define MAX_CMD_ARG     (4)

#define DLM             "\r\n"
#define PROMPT          DLM "# "

#define INBUF_SIZE      (1024)

struct console_command 
{
    const char *name;
    const char *help;
    void (*function) (int argc, char **argv);
};

static void help_command(int argc, char** argv);
static void burn_rtl8720_command(int argc, char** argv);
static void reset_factory_settings_command(int argc, char** argv);
static void display_settings_command(int argc, char** argv);
static void wifissid_command(int argc, char** argv);
static void wifipwd_command(int argc, char** argv);
static void az_idscope_command(int argc, char** argv);
static void az_regid_command(int argc, char** argv);
static void az_symkey_command(int argc, char** argv);
static void az_iotc_command(int argc, char** argv);

static const struct console_command cmds[] = 
{
  {"help"                  , "Help document"                                  , help_command                   },
  {"burn_rtl8720"          , "Enter the Burn RTL8720 Firmware mode"           , burn_rtl8720_command           },
  {"reset_factory_settings", "Reset factory settings"                         , reset_factory_settings_command },
  {"show_settings"         , "Display settings"                               , display_settings_command       },
  {"set_wifissid"          , "Set Wi-Fi SSID"                                 , wifissid_command               },
  {"set_wifipwd"           , "Set Wi-Fi password"                             , wifipwd_command                },
  {"set_az_idscope"        , "Set id scope of Azure IoT DPS"                  , az_idscope_command             },
  {"set_az_regid"          , "Set registration id of Azure IoT DPS"           , az_regid_command               },
  {"set_az_symkey"         , "Set symmetric key of Azure IoT DPS"             , az_symkey_command              },
  {"set_az_iotc"           , "Set connection information of Azure IoT Central", az_iotc_command                }
};

static const int cmd_count = sizeof(cmds) / sizeof(cmds[0]);

static void EnterBurnRTL8720Mode()
{
    // Switch mode of RTL8720
    pinMode(PIN_SERIAL2_RX, OUTPUT);
    pinMode(RTL8720D_CHIP_PU, OUTPUT);
    digitalWrite(PIN_SERIAL2_RX, LOW);
    digitalWrite(RTL8720D_CHIP_PU, LOW);
    delay(500);
    pinMode(RTL8720D_CHIP_PU, INPUT);
    delay(500);
    pinMode(PIN_SERIAL2_RX, INPUT);

    // Initialize UART
    Serial.beginWithoutDTR(115200);
    auto oldBaud = Serial.baud();
    RTL8720D.begin(oldBaud);
    delay(500);

    while (true)
    {
        // Change baud
        const auto baud = Serial.baud();
        if (baud != oldBaud)
        {
            RTL8720D.begin(baud);
            oldBaud = baud;
        }

        // USB -> RTL
        while (Serial.available()) RTL8720D.write(Serial.read());

        // RTL -> USB
        while (RTL8720D.available()) Serial.write(RTL8720D.read());
    }
}

static void print_help()
{
    Serial.print("Configuration console:" DLM);
    
    for (int i = 0; i < cmd_count; i++)
    {
        Serial.print(String::format(" - %s: %s." DLM, cmds[i].name, cmds[i].help));
    }
}

static void help_command(int argc, char** argv)
{
    print_help();
}

static void burn_rtl8720_command(int argc, char** argv)
{
    Serial.print("Enter the Burn RTL8720 Firmware mode." DLM);
    Serial.print("[Windows]" DLM);
    Serial.print("  ambd_flash_tool.exe erase" DLM);
    Serial.print("  ambd_flash_tool.exe flash -d [RTL8720-firmware-path]" DLM);
    Serial.print("[macOS/Linux]" DLM);
    Serial.print("  python3 ambd_flash_tool.py erase" DLM);
    Serial.print("  python3 ambd_flash_tool.py flash -d [RTL8720-firmware-path]" DLM);
    delay(1000);

    EnterBurnRTL8720Mode();
}

static void reset_factory_settings_command(int argc, char** argv)
{
    Storage::Erase();
    Storage::Load();

    Serial.print("Reset factory settings successfully." DLM);
}

static void display_settings_command(int argc, char** argv)
{
    Serial.print(String::format("Wi-Fi SSID = %s" DLM, Storage::WiFiSSID.c_str()));
    Serial.print(String::format("Wi-Fi password = %s" DLM, Storage::WiFiPassword.c_str()));
    Serial.print(String::format("Id scope of Azure IoT DPS = %s" DLM, Storage::IdScope.c_str()));
    Serial.print(String::format("Registration id of Azure IoT DPS = %s" DLM, Storage::RegistrationId.c_str()));
    Serial.print(String::format("Symmetric key of Azure IoT DPS = %s" DLM, Storage::SymmetricKey.c_str()));
}

static void wifissid_command(int argc, char** argv)
{
    if (argc != 2) 
    {
        Serial.print(String::format("ERROR: Usage: %s <SSID>. Please provide the SSID of the Wi-Fi." DLM, argv[0]));
        return;
    }

    Storage::WiFiSSID = argv[1];
    Storage::Save();

    Serial.print("Set Wi-Fi SSID successfully." DLM);
}

static void wifipwd_command(int argc, char** argv)
{
    if (argc != 2) 
    {
        Serial.print(String::format("ERROR: Usage: %s <Password>. Please provide the password of the Wi-Fi." DLM, argv[0]));
        return;
    }

    Storage::WiFiPassword = argv[1];
    Storage::Save();

    Serial.print("Set Wi-Fi password successfully." DLM);
}

static void az_idscope_command(int argc, char** argv)
{
    if (argc != 2) 
    {
        Serial.print(String::format("ERROR: Usage: %s <Id scope>. Please provide the id scope of the Azure IoT DPS." DLM, argv[0]));
        return;
    }

    Storage::IdScope = argv[1];
    Storage::Save();

    Serial.print("Set id scope successfully." DLM);
}

static void az_regid_command(int argc, char** argv)
{
    if (argc != 2) 
    {
        Serial.print(String::format("ERROR: Usage: %s <Registration id>. Please provide the registraion id of the Azure IoT DPS." DLM, argv[0]));
        return;
    }

    Storage::RegistrationId = argv[1];
    Storage::Save();

    Serial.print("Set registration id successfully." DLM);
}

static void az_symkey_command(int argc, char** argv)
{
    if (argc != 2) 
    {
        Serial.print(String::format("ERROR: Usage: %s <Symmetric key>. Please provide the symmetric key of the Azure IoT DPS." DLM, argv[0]));
        return;
    }

    Storage::SymmetricKey = argv[1];
    Storage::Save();

    Serial.print("Set symmetric key successfully." DLM);
}

static void az_iotc_command(int argc, char** argv)
{
    if (argc != 4) 
    {
        Serial.print(String::format("ERROR: Usage: %s <Id scope> <SAS key> <Device id>." DLM, argv[0]));
        return;
    }

    Storage::IdScope = argv[1];
    Storage::RegistrationId = argv[3];
    Storage::SymmetricKey = ComputeDerivedSymmetricKey(argv[2], argv[3]);
    Storage::Save();

    Serial.print("Set connection information of Azure IoT Central successfully." DLM);
}

static bool CliGetInput(char* inbuf, int* bp)
{
    if (inbuf == NULL) 
    {
        return false;
    }
    
    while (Serial.available() >= 1) 
    {
        inbuf[*bp] = (char)Serial.read();
        
        if (inbuf[*bp] == END_CHAR) 
        {
            /* end of input line */
            inbuf[*bp] = '\0';
            *bp = 0;
            return true;
        }
        else if (inbuf[*bp] == TAB_CHAR) 
        {
            inbuf[*bp] = SPACE_CHAR;
        }
        else if (inbuf[*bp] == BACKSPACE_CHAR || inbuf[*bp] == DEL_CHAR)
        {
            // Delete
            if (*bp > 0) 
            {
                (*bp)--;
                Serial.write(BACKSPACE_CHAR);
                Serial.write(SPACE_CHAR);
                Serial.write(BACKSPACE_CHAR);
            }
            continue;
        }
        else if (inbuf[*bp] < SPACE_CHAR)
        {
            continue;
        }

        // Echo
        Serial.write(inbuf[*bp]);
        (*bp)++;
        
        if (*bp >= INBUF_SIZE) 
        {
            Serial.print(DLM "ERROR: Input buffer overflow." DLM);
            Serial.print(PROMPT);
            *bp = 0;
            continue;
        }
    }
    
    return false;
}

static bool CliHandleInput(char* inbuf)
{
    struct
    {
        unsigned inArg:1;
        unsigned inQuote:1;
        unsigned done:1;
    } stat;
  
    char* argv[MAX_CMD_ARG];
    int argc = 0;

    int i = 0;
        
    memset((void*)&argv, 0, sizeof(argv));
    memset(&stat, 0, sizeof(stat));
  
    do 
    {
        switch (inbuf[i]) 
        {
        case '\0':
            if (stat.inQuote)
            {
                return false;
            }
            stat.done = 1;
            break;
  
        case '"':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) 
            {
                memcpy(&inbuf[i - 1], &inbuf[i], strlen(&inbuf[i]) + 1);
                
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg) break;
            if (stat.inQuote && !stat.inArg) return false;
            
            if (!stat.inQuote && !stat.inArg) 
            {
                stat.inArg = 1;
                stat.inQuote = 1;
                argc++;
                argv[argc - 1] = &inbuf[i + 1];
            } 
            else if (stat.inQuote && stat.inArg) 
            {
                stat.inArg = 0;
                stat.inQuote = 0;
                inbuf[i] = '\0';
            }
            break;
      
        case ' ':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) 
            {
                memcpy(&inbuf[i - 1], &inbuf[i], strlen(&inbuf[i]) + 1);
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg)
            {
                stat.inArg = 0;
                inbuf[i] = '\0';
            }
            break;
        default:
            if (!stat.inArg) 
            {
                stat.inArg = 1;
                argc++;
                argv[argc - 1] = &inbuf[i];
            }
            break;
        }
    }
    while (!stat.done && ++i < INBUF_SIZE && argc <= MAX_CMD_ARG);
  
    if (stat.inQuote) return false;
    if (argc < 1) return true;
    
    Serial.print(DLM);
    
    for(int i = 0; i < cmd_count; i++)
    {
        if(strcmp(cmds[i].name, argv[0]) == 0)
        {
            cmds[i].function(argc, argv);
            return true;
        }
    }
    
    Serial.print(String::format("ERROR: Invalid command: %s" DLM, argv[0]));
    return true;
}

void CliMode()
{
    print_help();
    Serial.print(PROMPT);

    char inbuf[INBUF_SIZE];
    int bp = 0;
    while (true) 
    {
        if (!CliGetInput(inbuf, &bp)) continue;

        if (!CliHandleInput(inbuf))
        {
            Serial.print("ERROR: Syntax error." DLM);
        }

        Serial.print(PROMPT);
    }
}
