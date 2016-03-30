#include <3ds.h>

// Code from examples/3ds/http/source/main.c
Result http_download(httpcContext *context, u8* out)//This error handling needs updated with proper text printing once ctrulib itself supports that.
{
	Result ret=0;
	u8* framebuf_top;
	u32 statuscode=0;
	u32 contentsize=0;
	u8 *buf;

	ret = httpcBeginRequest(context);
	if(ret!=0)return ret;

	ret = httpcGetResponseStatusCode(context, &statuscode, 0);
	if(ret!=0)return ret;

	if(statuscode!=200)return -2;

	ret=httpcGetDownloadSizeState(context, NULL, &contentsize);
	if(ret!=0)return ret;

	gfxFlushBuffers();

	buf = out;
	if(buf==NULL)return -1;
	memset(buf, 0, contentsize);


	ret = httpcDownloadData(context, buf, contentsize, NULL);
	if(ret!=0)
	{
		free(buf);
		return ret;
	}

	free(buf);

	return 0;
}

char* vstr_from_kver(u32 kver)
{
    u8 v_major = GET_VERSION_MAJOR(kver);
    u8 v_minor = GET_VERSION_MINOR(kver);
    u8 v_build = GET_VERSION_REVISION(kver);

    switch(v_major)
    {
    case 2:
        switch(v_minor)
        {
        case 3:
            return "Factory FIRM";
        case 27:
            return "1.0.0";
        case 28:
            return "1.1.0";
        case 29:
            return "2.0.0";
        case 30:
            return "2.1.0";
        case 31:
            return "2.2.0";
        case 32:
            return "3.0.0";
        case 33:
            return "4.0.0";
        case 34:
            return "4.1.0";
        case 35:
            return "5.0.0";
        case 36:
            return "5.1.0";
        case 37:
            return "6.0.0";
        case 38:
            return "6.1.0";
        case 39:
            return "7.0.0";
        case 40:
            return "7.2.0";
        case 44:
            return "8.0.0";
        case 45:
            return "8.1.0";
        case 46:
            return "9.0.0";
        case 48:
            return "9.3.0";
        case 49:
            return "9.5.0";
        case 50:
            switch(v_build)
            {
            case 1:
                return "9.6.0";
            case 7:
                return "10.0.0";
            case 9:
                return "10.2.0";
            case 11:
                return "10.4.0";
            default:
                return "Unknown";
            }
        default:
            return "Unknown";
        }
    default:
        return "Unknown";
    }
}

int main(int argc, char** argv)
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    aptInit();
    httpcInit(0);

    printf("Ready to install notihax! Press A to install, press any other key to exit.\n");

    while(aptMainLoop())
    {
        hidScanInput();

        if(hidKeysDown())
        {
            if(hidKeysDown() & KEY_A) {
                break;
            } else {
                goto exit;
            }
        }
    }

    u8 n3ds;
    APT_CheckNew3DS(&n3ds);

    u32 kver = osGetKernelVersion();
    printf("kver:      %8x\n", kver);
    printf("sysverraw: %1u %2u %2u\n", GET_VERSION_MAJOR(kver), GET_VERSION_MINOR(kver), GET_VERSION_REVISION(kver));

    char* payload_loc;
    sprintf(payload_loc, "http://raw.githubusercontent.com/TechNick6425/notihax/master/payloads/%s%8x", (n3ds != 0 ? "n" : "o"), kver);

    printf("Downloading payload...\n");

    httpcContext context;
    httpcOpenContext(&context, HTTPC_METHOD_GET, payload_loc, 1);

    

exit:
    aptExit();
    httpcExit();
	gfxExit();
}
