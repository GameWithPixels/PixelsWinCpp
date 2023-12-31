/**
 * @file
 * @brief Definition of the Logger internal class.
 */

#pragma once

#include <fstream>
#include <sstream>
#include <thread>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <filesystem>

namespace Systemic::Internal
{
    /**
     * @brief Simple logging class, meant to be used for debugging.
     */
    class Logger
    {
    public:
        /**
         * @brief Log the given message.
         * @param message The message to log.
         */
        static void log(const std::string& message)
        {
            static std::ofstream myfile(std::filesystem::temp_directory_path().append("systemic_log.txt").native());

            std::ostringstream ss;
            time_in_HH_MM_SS_MMM(ss);
            ss << "[" << std::this_thread::get_id() << "]" << ": " << message << "\n";
            myfile << ss.str();
            myfile.flush();
        }

    private:
        static void time_in_HH_MM_SS_MMM(std::ostream& out)
        {
            using namespace std::chrono;

            // get current time
            auto now = system_clock::now();

            // get number of milliseconds for the current second
            // (remainder after division into seconds)
            auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

            // convert to std::time_t in order to convert to std::tm (broken time)
            auto timer = system_clock::to_time_t(now);

            // convert to broken time
            std::tm bt;
            ::localtime_s(&bt, &timer);

            // save default formatting
            std::ios init(nullptr);
            init.copyfmt(out);

            out << std::put_time(&bt, "%H:%M:%S"); // HH:MM:SS
            out << '.' << std::setfill('0') << std::setw(3) << ms.count() << std::setw(3);

            // restore default formatting
            out.copyfmt(init);
        }
    };
}
