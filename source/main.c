#include <3ds.h>

int main(int argc, char** argv)
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("Ready to install notihax! Press A to install, press any other key to exit.\n");

    while(aptMainLoop())
    {
        hidScanInput();

        if(hidKeysDown())
        {
            if(hidKeysDown() & KEY_A) {
                break;
            } else {
                return 0;
            }
        }
    }


    printf("FIRM: %x", osGetFirmVersion());

    while(aptMainLoop())
    {
        hidScanInput();

        if(hidKeysDown())
        {
            return 0;
        }
    }
}
