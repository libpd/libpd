#include "Gem/GemConfig.h"

#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#else
# include <glob.h>
#endif

#include "Files.h"

#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif

#include "Gem/RTE.h"

namespace gem {
  namespace files {

    std::vector<std::string>getFilenameListing(const std::string&pattern) {
      std::vector<std::string>result;
#ifdef _WIN32
      WIN32_FIND_DATA findData;
      DWORD errorNumber;
      HANDLE hFind;
      LPVOID lpErrorMessage;
      
      hFind = FindFirstFile(pattern.c_str(), &findData);
      if (hFind == INVALID_HANDLE_VALUE) 
        {
          errorNumber = GetLastError();
          switch (errorNumber)
            {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
              break;
            default:
              FormatMessage(
                            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                            FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            errorNumber,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (LPTSTR) &lpErrorMessage,
                            0, NULL );
              //pd_error(x,"[folder_list] %s", (char *)lpErrorMessage);
            }
          return result;
        }
      do {
        result.push_back(findData.cFileName);
      } while (FindNextFile(hFind, &findData) != 0);

      FindClose(hFind);
#else
      /* use glob */
      glob_t glob_buffer;
      int i=0;
      switch( glob( pattern.c_str(), GLOB_TILDE, NULL, &glob_buffer ) ) {
      case GLOB_NOSPACE: 
        //        error("out of memory for \"%s\"",pattern.c_str()); 
        return result;
# ifdef GLOB_ABORTED
      case GLOB_ABORTED: 
        //error("aborted \"%s\"",pattern.c_str()); 
        return result;
# endif
# ifdef GLOB_NOMATCH
      case GLOB_NOMATCH: 
        //error("nothing found for \"%s\"",pattern.c_str()); 
        return result;
# endif
      }
      for(i=0; i<glob_buffer.gl_pathc; i++) {
        result.push_back(glob_buffer.gl_pathv[i]);
      }
      globfree( &(glob_buffer) );
#endif

      return result;
    }


    std::string expandEnv(const std::string&value, bool bashfilename) {
      std::string ret;
      /* FIXXME: 
       *  ouch, on linux value has to include "$VARIABLENAME", check whether we need "%VARIABLENAME%" on w32
       */

      if(value.empty())
        return value;

      if(bashfilename) {
        char bashBuffer[MAXPDSTRING];
        sys_bashfilename(value.c_str(), bashBuffer);
        ret=bashBuffer;
      } else {
        ret=value;
      }

#ifdef HAVE_WORDEXP_H
      wordexp_t pwordexp;
      
      if(0==wordexp(ret.c_str(), &pwordexp, 0)) {
        pwordexp.we_offs=0;
        if(pwordexp.we_wordc) {
          // we only take the first match into account 
          ret=pwordexp.we_wordv[0];
        }
# ifdef __APPLE__
        /* wordfree() broken on apple: keeps deallocating non-aligned memory */
#  warning wordfree() not called
# else
        wordfree(&pwordexp);
# endif
      }
#endif
#ifdef _WIN32
      char envVarBuffer[MAXPDSTRING];
      ExpandEnvironmentStrings(ret.c_str(), envVarBuffer, MAX_PATH - 2);
      ret=envVarBuffer;
#endif
      return ret;
    }


    std::string getExtension(const std::string&fileName, const bool lower) {
      using namespace std;
      std::string Ext;
      std::string::size_type idx;
      idx = fileName.rfind('.');
    
      if(idx != std::string::npos)
        Ext=fileName.substr(idx + 1);
     
      if(lower) {
        // lower-case: http://www.cplusplus.com/forum/general/837/
#if 0
        std::transform(Ext.begin(), Ext.end(), Ext.begin(), std::tolower);
#else
        const int length = Ext.length();
        for(int i=0; i < length; ++i)  {
          Ext[i] = tolower(Ext[i]);
        }
#endif
      }
      return Ext;
    }



  };
};
