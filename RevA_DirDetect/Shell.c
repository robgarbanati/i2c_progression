// Include header files
#include <string.h>
#include "Platform.h"
#include "Commu/SoftUART.h"
#include "XPrintf.h"
#include "SpiFS.h"
#include "Shell.h"
#include "DirDetect.h"

#define CMDLINE_SIZE	 		64

static INT32 shellDir(void)
{
	SPIFS_DIR dir;

	// Open the file system directory.
	if (spiFSOpenDir(&dir))
	{
		// Read each filename from the file system directory.
		while (spiFSReadDir(&dir))
		{
			// Print the filename.
			xprintf("%-18s %10d\n", dir.fileName, dir.fileSize);
		}
		
		// Close the the file system directory.
		spiFSCloseDir(&dir);
	}
	
	return 0;
}

static INT32 shellFormat(void)
{
 	xputs("Formatting SPI Flash disk\n");

	// Format the the SPI flash device.
	spiFSFormat();

	return 0;
}

static INT32 shellUnlink(char *filename)
{
	spiFSUnlink(filename);

	return 0;
}

static INT32 shellReadTest(char *filename)
{
	INT16 i;
	INT16 length;
	char buffer[16];
  SPIFS_FILE file;

	// Open the file.
	if (spiFSOpen(filename, FALSE, &file))
	{
		length = sizeof(buffer);

		// Read data from the file.
		while (spiFSRead(&file, (UINT8 *) buffer, &length))
		{
			// Write the buffer out.
			for (i = 0; i < length; ++i) xputc(buffer[i]);
		}

		// Close the file.
		spiFSClose(&file);
	}

	return 0;
}

static INT32 shellWriteTest(char *filename)
{
	INT16 i;
	INT16 length;
	char buffer[64];
  SPIFS_FILE file;

	// Open the file.
	if (spiFSOpen(filename, TRUE, &file))
	{
		// Write lines to the file.
		for (i = 1; i <= 1000; ++i)
		{
			xsprintf(buffer, "Test line %d\n", (int) i);
			length = (UINT16) strlen(buffer);
			spiFSWrite(&file, (UINT8 *) buffer, &length);
		}

		// Close the file.
		spiFSClose(&file);
	}

	return 0;
}

void shell(void)
{
	// Output the prompt.
	xputs("\r\n\r\nStart Shell\r\n");

	// Command interpreter loop.
	for (;;)
	{
		char *cmd;
		char *arg1;
		char *arg2;
		char *cmdLine;
		static char commandLine[CMDLINE_SIZE];
		INT32 status = 0;

		// Output the prompt.
		xputs("EVB> ");

		// Read a line of input as upper case letters.
		xgets(commandLine, sizeof(commandLine), 1);

		// Point to the command line.
		cmdLine = commandLine;

		// Parse the command and any arguments.
		xparse(&cmdLine, &cmd);
		xparse(&cmdLine, &arg1);
		xparse(&cmdLine, &arg2);

		if (!strcmp(cmd, "DIR"))
		{
			status = shellDir();
		}
		else if (!strcmp(cmd, "READ"))
		{
			status = shellReadTest(arg1);
		}
		else if (!strcmp(cmd, "WRITE"))
		{
			status = shellWriteTest(arg1);
		}
		else if (!strcmp(cmd, "DEL"))
		{
			status = shellUnlink(arg1);
		}
		else if (!strcmp(cmd, "FORMAT"))
		{
			status = shellFormat();
		}
		else if (!strcmp(cmd, "DIRECTION"))
		{
			xputs("switch success\n");
			signalSettingFunction();
		}
		else if (!strcmp(cmd, "Q"))
		{
			return;
		}
		
		// TODO ADD A COMMAND

		// Should we output an error?
		if (status != 0)
		{
			xprintf("Command line error, error code %d\n", status);
		}
	}
}

