
// STL includes
#include <csignal>
#include <iomanip>

// QT includes
#include <QImage>

// getoptPlusPLus includes
#include <getoptPlusPlus/getoptpp.h>

// Dispmanx grabber includes
#include <dispmanx-grabber/DispmanxFrameGrabber.h>

using namespace vlofgren;

static bool running = true;

void signal_handler(int signum)
{
	running = false;
}

int main(int argc, char** argv)
{
	signal(SIGTERM, signal_handler);
	signal(SIGINT,  signal_handler);

	int grabFlags = 0;
	try
	{
		// create the option parser and initialize all parameters
		OptionsParser optionParser("Simple application to send a command to hyperion using the Json interface");
		ParameterSet & parameters = optionParser.getParameters();

		QString flagDescr = QString("Set the grab flags of the dispmanx frame grabber [default: 0x%1]").arg(grabFlags, 8, 16, QChar('0'));
		StringParameter   & argFlags = parameters.add<StringParameter>   ('f', "flags", flagDescr.toAscii().constData());
		SwitchParameter<> & argList  = parameters.add<SwitchParameter<> >('l', "list",  "List the possible flags");
		SwitchParameter<> & argHelp  = parameters.add<SwitchParameter<> >('h', "help",  "Show this help message and exit");

		// parse all options
		optionParser.parse(argc, const_cast<const char **>(argv));

		// check if we need to display the usage. exit if we do.
		if (argHelp.isSet())
		{
			optionParser.usage();
			return 0;
		}
		if (argList.isSet())
		{
			std::cout.width(15);
			std::cout.width(10);
			std::cout << "Possible DISPMANX flags: " << std::endl;
			std::cout << "Name                            | " << "Value" << std::endl;
			std::cout << "--------------------------------| " << "------" << std::endl;
			std::cout << "DISPMANX_NO_ROTATE              | 0x" << std::hex << std::setfill('0') << std::setw(8) << DISPMANX_NO_ROTATE << std::endl;
			std::cout << "DISPMANX_ROTATE_90              | 0x" << std::hex << std::setfill('0') << std::setw(8) << DISPMANX_ROTATE_90 << std::endl;
			std::cout << "DISPMANX_ROTATE_180             | 0x" << std::hex << std::setfill('0') << std::setw(8) << DISPMANX_ROTATE_180 << std::endl;
			std::cout << "DISPMANX_ROTATE_270             | 0x" << std::hex << std::setfill('0') << std::setw(8) << DISPMANX_ROTATE_270 << std::endl;

			std::cout << "DISPMANX_FLIP_HRIZ              | 0x" << std::hex << std::setfill('0') << std::setw(8) << DISPMANX_FLIP_HRIZ << std::endl;
			std::cout << "DISPMANX_FLIP_VERT              | 0x" << std::hex << std::setfill('0') << std::setw(8) << DISPMANX_FLIP_VERT << std::endl;

			std::cout << "DISPMANX_SNAPSHOT_NO_YUV        | 0x" << std::hex << std::setfill('0') << std::setw(8) << DISPMANX_SNAPSHOT_NO_YUV << std::endl;
			std::cout << "DISPMANX_SNAPSHOT_NO_RGB        | 0x" << std::hex << std::setfill('0') << std::setw(8) << DISPMANX_SNAPSHOT_NO_RGB << std::endl;
			std::cout << "DISPMANX_SNAPSHOT_FILL          | 0x" << std::hex << std::setfill('0') << std::setw(8) << DISPMANX_SNAPSHOT_FILL << std::endl;
			std::cout << "DISPMANX_SNAPSHOT_SWAP_RED_BLUE | 0x" << std::hex << std::setfill('0') << std::setw(8) << DISPMANX_SNAPSHOT_SWAP_RED_BLUE << std::endl;
			std::cout << "DISPMANX_SNAPSHOT_PACK          | 0x" << std::hex << std::setfill('0') << std::setw(8) << DISPMANX_SNAPSHOT_PACK << std::endl;
			return 0;
		}
		if (argFlags.isSet())
		{
			QString flagStr = QString::fromStdString(argFlags.getValue());

			bool ok = false;
//			grabFlags = flagStr.toInt(&ok);
			if (flagStr.startsWith("0x"))
			{
				grabFlags = flagStr.toInt(&ok, 16);
			}
			else
			{
				grabFlags = flagStr.toInt(&ok, 10);
			}
			std::cout << "Resulting flags: " << grabFlags << " (=0x" << std::hex << std::setfill('0') << std::setw(8) << grabFlags << ")" << std::dec << std::endl;
			if (!ok)
			{
				std::cerr << "Failed to parse flags (" << flagStr.toStdString().c_str() << ")" << std::endl;
				return -1;
			}
		}
	}
	catch (const std::runtime_error & e)
	{
		// An error occured. Display error and quit
		std::cerr << e.what() << std::endl;
		return 1;
	}


	DispmanxFrameGrabber frameGrabber(64, 64);
	frameGrabber.setFlags(grabFlags);

	unsigned iFrame = 0;
	QImage qImage(64, 64, QImage::Format_ARGB32);
	Image<ColorRgba> imageRgba(64, 64);

	while(running)
	{
		frameGrabber.grabFrame(imageRgba);

		for (int iScanline=0; iScanline<qImage.height(); ++iScanline)
		{
			unsigned char* scanLinePtr = qImage.scanLine(iScanline);
			memcpy(scanLinePtr, imageRgba.memptr()+imageRgba.width()*iScanline, imageRgba.width()*sizeof(ColorRgba));
		}

		qImage.save(QString("HYPERION_%3.png").arg(iFrame));
		++iFrame;

		timespec sleepTime;
		sleepTime.tv_sec  = 1;
		sleepTime.tv_nsec = 0;
		nanosleep(&sleepTime, NULL);
	}

	return 0;
}
