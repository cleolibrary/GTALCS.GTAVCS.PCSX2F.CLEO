#include "utils.h"
#include "mutex.h"
#include "core.h"

#include "../includes/pcsx2/log.h"

namespace utils
{
	uint32_t get_tick_count()
	{
		return *core::CTimer__m_snTimeInMilliseconds;
	}

	//FILE *log_file = NULL;

	void log(const char *fmt, ...)
	{
		//uncomment for log
		//CS_SCOPE(mutex::mlog);
		//
		//char str[256];
		//va_list lst;
		//va_start(lst, fmt);
		//vsprintf(str, fmt, lst);
		//va_end(lst);
		//
		//logger.WriteF("%s", str);
	}

	bool string_compare(const std::string &left, const std::string &right)
	{
	   for (std::string::const_iterator lit = left.begin(), rit = right.begin(); lit != left.end() && rit != right.end(); ++lit, ++rit)
	      if (tolower(*lit) < tolower(*rit))
	         return true;
	      else if (tolower(*lit) > tolower(*rit))
	         return false;
	   if (left.size() < right.size())
	      return true;
	   return false;
	}

	bool list_files_in_dir(std::string dir, std::vector<std::string> &files)
	{
		//int fd = sceIoDopen(dir.c_str());
		//if (fd < 0) return false;
		//SceIoDirent dirp;
		//memset(&dirp, 0, sizeof(dirp));
		//while (sceIoDread(fd, &dirp) > 0)
		//	if((dirp.d_stat.st_attr & FIO_SO_IFDIR) == 0)
		//		files.push_back(std::string(dirp.d_name));
		//sceIoDclose(fd);
		//
	    //if (files.size())
	    //	sort(files.begin(), files.end(), string_compare);
	    //return true;
	}

	uint8_t *load_binary_file(std::string filename, uint32_t &size)
	{
		//FILE *file = fopen(filename.c_str(), "r");
		//if (!file) return NULL;
		//fseek(file, 0, SEEK_END);
		//size = ftell(file);
		//fseek(file, 0, SEEK_SET);
		//uint8_t *buf = cast<ptr>(malloc(size));
		//fread(buf, 1, size, file);
		//fclose(file);
		//return buf;
	}

}


