/**
 * \file debug.cxx
 * \brief This file contains the definitions of debug related objects and functions.
 *
 * Copyright (C) 2016 Carlo Wood.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sys.h"                        // Needed for platform-specific code

#ifdef CWDEBUG

#include <cctype>                       // Needed for std::isprint
#include <cstdio>                       // Needed for sprintf
#include <iomanip>                      // Needed for setfill
#include <map>
#include <string>
#include <sstream>
#include "debug.h"
#ifdef USE_LIBCW
#include <libcw/memleak.h>		// memleak_filter
#endif

#if LIBCWD_THREAD_SAFE
pthread_mutex_t cout_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

namespace NAMESPACE_DEBUG {
  namespace NAMESPACE_CHANNELS {
    namespace dc {

#ifndef DOXYGEN
#define DDCN(x) (x)
#endif
      // Add new debug channels here.

      channel_ct xmlparser DDCN("XMLPARSER");   //!< This debug channel is used for parsing xml files.
      channel_ct profiler  DDCN("PROFILER");    //!< This debug channel is used for profiling the jack client.

    } // namespace dc
  } // namespace DEBUGCHANNELS

  // Anonymous namespace, this map and its initialization functions are private to this file
  // for Thead-safeness reasons.
  namespace {

    /*! @brief The type of rcfile_dc_states.
     * @internal
     */
    typedef std::map<std::string, bool> rcfile_dc_states_type;

    /*! @brief Map containing the default debug channel states used at the start of each new thread.
     * @internal
     *
     * The first thread calls main, which calls debug::init which will initialize this
     * map with all debug channel labels and whether or not they were turned on in the
     * rcfile or not.
     */
    rcfile_dc_states_type rcfile_dc_states;

    /*! @brief Set the default state of debug channel \a dc_label.
     * @internal
     *
     * This function is called once for each debug channel.
     */
    void set_state(char const* dc_label, bool is_on)
    {
      std::pair<rcfile_dc_states_type::iterator, bool> res =
          rcfile_dc_states.insert(rcfile_dc_states_type::value_type(std::string(dc_label), is_on));
      if (!res.second)
        Dout(dc::warning, "Calling set_state() more than once for the same label!");
      return;
    }

    /*! @brief Save debug channel states.
     * @internal
     *
     * One time initialization function of rcfile_dc_state.
     * This must be called from debug::init after reading the rcfile.
     */
    void save_dc_states()
    {
      // We may only call this function once: it reflects the states as stored
      // in the rcfile and that won't change.  Therefore it is not needed to
      // lock `rcfile_dc_states', it is only written to by the first thread
      // (once, via main -> init) when there are no other threads yet.
      static bool second_time = false;
      if (second_time)
      {
        Dout(dc::warning, "Calling save_dc_states() more than once!");
	return;
      }
      second_time = true;
      ForAllDebugChannels( set_state(debugChannel.get_label(), debugChannel.is_on()) );
    }

  } // anonymous namespace

  /*! @brief Returns the the original state of a debug channel.
   * @internal
   *
   * For a given \a dc_label, which must be the exact name (<tt>channel_ct::get_label</tt>) of an
   * existing debug channel, this function returns \c true when the corresponding debug channel was
   * <em>on</em> at the startup of the application, directly after reading the libcwd runtime
   * configuration file (.libcwdrc).
   *
   * If the label/channel did not exist at the start of the application, it will return \c false
   * (note that libcwd disallows adding debug channels to modules - so this would probably
   * a bug).
   */
  bool is_on_in_rcfile(char const* dc_label)
  {
    rcfile_dc_states_type::const_iterator iter = rcfile_dc_states.find(std::string(dc_label));
    if (iter == rcfile_dc_states.end())
    {
      Dout(dc::warning, "is_on_in_rcfile(\"" << dc_label << "\"): \"" << dc_label << "\" is an unknown label!");
      return false;
    }
    return iter->second;
  }

  /*! @brief Initialize debugging code from new threads.
   *
   * This function needs to be called at the start of each new thread,
   * because a new thread starts in a completely reset state.
   *
   * The function turns on all debug channels that were turned on
   * after reading the rcfile at the start of the application.
   * Furthermore it initializes the debug ostream, its mutex and the
   * margin of the default debug object (Dout).
   */
  void init_thread()
  {
    // Turn on all debug channels that are turned on as per rcfile configuration.
    ForAllDebugChannels(
        if (!debugChannel.is_on() && is_on_in_rcfile(debugChannel.get_label()))
	  debugChannel.on();
    );

    // Turn on debug output.
    Debug( libcw_do.on() );
#if LIBCWD_THREAD_SAFE
    Debug( libcw_do.set_ostream(&std::cout, &cout_mutex) );
#else
    Debug( libcw_do.set_ostream(&std::cout) );
#endif

    static bool first_thread = true;
    if (!first_thread)			// So far, the application has only one thread.  So don't add a thread id.
    {
      // Set the thread id in the margin.
      char margin[12];
      sprintf(margin, "%-10lu ", pthread_self());
      Debug( libcw_do.margin().assign(margin, 11) );
    }
  }

  /*! @brief Initialize debugging code from main.
   *
   * This function initializes the debug code.
   */
  void init()
  {
#if CWDEBUG_ALLOC && defined(USE_LIBCW)
    // Tell the memory leak detector which parts of the code are
    // expected to leak so that we won't get an alarm for those.
    {
      std::vector<std::pair<std::string, std::string> > hide_list;
      hide_list.push_back(std::pair<std::string, std::string>("libdl.so.2", "_dlerror_run"));
      hide_list.push_back(std::pair<std::string, std::string>("libstdc++.so.6", "__cxa_get_globals"));
      // The following is actually necessary because of a bug in glibc
      // (see http://sources.redhat.com/bugzilla/show_bug.cgi?id=311).
      hide_list.push_back(std::pair<std::string, std::string>("libc.so.6", "dl_open_worker"));
      memleak_filter().hide_functions_matching(hide_list);
    }
    {
      std::vector<std::string> hide_list;
      // Also because of http://sources.redhat.com/bugzilla/show_bug.cgi?id=311
      hide_list.push_back(std::string("ld-linux.so.2"));
      memleak_filter().hide_objectfiles_matching(hide_list);
    }
    memleak_filter().set_flags(libcwd::show_objectfile|libcwd::show_function);
#endif

#ifndef NO_SYNC_WITH_STDIO
    // The following call allocated the filebuf's of cin, cout, cerr, wcin, wcout and wcerr.
    // Because this causes a memory leak being reported, make them invisible.
    Debug(set_invisible_on());

    // You want this, unless you mix streams output with C output.
    // Read  http://gcc.gnu.org/onlinedocs/libstdc++/27_io/howto.html#8 for an explanation.
    std::ios::sync_with_stdio(false);

    // Cancel previous call to set_invisible_on.
    Debug(set_invisible_off());
#endif

    // This will warn you when you are using header files that do not belong to the
    // shared libcwd object that you linked with.
    Debug( check_configuration() );

    Debug(
      libcw_do.on();		// Show which rcfile we are reading!
      ForAllDebugChannels(
        while (debugChannel.is_on())
	  debugChannel.off()	// Print as little as possible though.
      );
      read_rcfile();		// Put 'silent = on' in the rcfile to suppress most of the output here.
      libcw_do.off()
    );
    save_dc_states();

    init_thread();
  }

#if CWDEBUG_LOCATION
  /*! @brief Return call location.
   *
   * @param return_addr The return address of the call.
   */
  std::string call_location(void const* return_addr)
  {
    libcwd::location_ct loc((char*)return_addr + libcwd::builtin_return_address_offset);
    std::ostringstream convert;
    convert << loc;
    return convert.str();
  }
#endif

} // namespace NAMESPACE_DEBUG

#endif // CWDEBUG
