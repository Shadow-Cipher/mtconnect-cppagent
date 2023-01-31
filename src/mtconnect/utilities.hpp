//
// Copyright Copyright 2009-2022, AMT – The Association For Manufacturing Technology (“AMT”)
// All rights reserved.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//

#pragma once

#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/regex.hpp>

#include <chrono>
#include <date/date.h>
#include <mtconnect/version.h>

#include "mtconnect/config.hpp"

//####### CONSTANTS #######

// Port number to put server on
const unsigned int SERVER_PORT = 8080;

// Size of sliding buffer
const unsigned int DEFAULT_SLIDING_BUFFER_SIZE = 131072;

// Size of buffer exponent: 2^SLIDING_BUFFER_EXP
const unsigned int DEFAULT_SLIDING_BUFFER_EXP = 17;
const unsigned int DEFAULT_MAX_ASSETS = 1024;

/// @brief MTConnect namespace
///
/// Top level mtconnect namespace
namespace mtconnect {
  // Message for when enumerations do not exist in an array/enumeration
  const int ENUM_MISS = -1;

  /// @brief Time formats
  enum TimeFormat
  {
    HUM_READ, ///< Human readable
    GMT, ///< GMT or UTC with second resolution
    GMT_UV_SEC, ///< GMT with microsecond resolution
    LOCAL ///< Time using local time zone 
  };

  /// @brief Converts string to floating point numberss
  /// @param[in] text the number
  /// @return the converted value or 0.0 if incorrect.
  inline double stringToFloat(const std::string &text)
  {
    double value = 0.0;
    try
    {
      value = stof(text);
    }
    catch (const std::out_of_range &)
    {
      value = 0.0;
    }
    catch (const std::invalid_argument &)
    {
      value = 0.0;
    }
    return value;
  }

  /// @brief Converts string to integer
  /// @param[in] text the number
  /// @return the converted value or 0 if incorrect.
  inline int stringToInt(const std::string &text, int outOfRangeDefault)
  {
    int value = 0;
    try
    {
      value = stoi(text);
    }
    catch (const std::out_of_range &)
    {
      value = outOfRangeDefault;
    }
    catch (const std::invalid_argument &)
    {
      value = 0;
    }
    return value;
  }

  /// @brief converts a double to a string
  /// @param[in] value the double
  /// @return the string representation of the double (10 places max)
  inline std::string format(double value)
  {
    std::stringstream s;
    constexpr int precision = std::numeric_limits<double>::digits10;
    s << std::setprecision(precision) << value;
    return s.str();
  }

  /// @brief inline formattor support for doubles
  class format_double_stream
  {
  protected:
    double val;

  public:
    /// @brief create a formatter
    /// @param[in] v the value
    format_double_stream(double v) { val = v; }

    /// @brief writes a double to an output stream with up to 10 digits of precision
    /// @tparam _CharT from std::basic_ostream
    /// @tparam _Traits from std::basic_ostream
    /// @param[in,out] os output stream
    /// @param[in] fmter reference to this formatter
    /// @return reference to the output stream
    template <class _CharT, class _Traits>
    inline friend std::basic_ostream<_CharT, _Traits> &operator<<(
        std::basic_ostream<_CharT, _Traits> &os, const format_double_stream &fmter)
    {
      constexpr int precision = std::numeric_limits<double>::digits10;
      os << std::setprecision(precision) << fmter.val;
      return os;
    }
  };

  /// @brief create a `format_doulble_stream`
  /// @param[in] v the value
  /// @return the format_double_stream 
  inline format_double_stream formatted(double v) { return format_double_stream(v); }

  /// @brief Convert text to upper case
  /// @param[in,out] text text
  /// @return upper-case of text as string
  inline std::string toUpperCase(std::string &text)
  {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    return text;
  }

  // Check if each char in a string is a positive integer
  inline bool isNonNegativeInteger(const std::string &s)
  {
    for (const char c : s)
    {
      if (!isdigit(c))
        return false;
    }

    return true;
  }

  inline bool isInteger(const std::string &s)
  {
    auto iter = s.cbegin();
    if (*iter == '-' || *iter == '+')
      ++iter;

    for (; iter != s.end(); iter++)
    {
      if (!isdigit(*iter))
        return false;
    }

    return true;
  }

  AGENT_LIB_API void mt_localtime(const time_t *time, struct tm *buf);

  // Get a specified time formatted
  inline std::string getCurrentTime(std::chrono::time_point<std::chrono::system_clock> timePoint,
                                    TimeFormat format)
  {
    using namespace std;
    using namespace std::chrono;
    constexpr char ISO_8601_FMT[] = "%Y-%m-%dT%H:%M:%SZ";

    switch (format)
    {
      case HUM_READ:
        return date::format("%a, %d %b %Y %H:%M:%S GMT", date::floor<seconds>(timePoint));
      case GMT:
        return date::format(ISO_8601_FMT, date::floor<seconds>(timePoint));
      case GMT_UV_SEC:
        return date::format(ISO_8601_FMT, date::floor<microseconds>(timePoint));
      case LOCAL:
        auto time = system_clock::to_time_t(timePoint);
        struct tm timeinfo = {0};
        mt_localtime(&time, &timeinfo);
        char timestamp[64] = {0};
        strftime(timestamp, 50u, "%Y-%m-%dT%H:%M:%S%z", &timeinfo);
        return timestamp;
    }

    return "";
  }

  // Get the current time formatted
  inline std::string getCurrentTime(TimeFormat format)
  {
    return getCurrentTime(std::chrono::system_clock::now(), format);
  }

  template <class timePeriod>
  inline uint64_t getCurrentTimeIn()
  {
    return std::chrono::duration_cast<timePeriod>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
  }

  // time_t to the ms
  inline uint64_t getCurrentTimeInMicros() { return getCurrentTimeIn<std::chrono::microseconds>(); }

  inline uint64_t getCurrentTimeInSec() { return getCurrentTimeIn<std::chrono::seconds>(); }

  AGENT_LIB_API uint64_t parseTimeMicro(const std::string &aTime);

  // Replace illegal XML characters with the correct corresponding characters
  inline void replaceIllegalCharacters(std::string &data)
  {
    for (auto i = 0u; i < data.length(); i++)
    {
      char c = data[i];

      switch (c)
      {
        case '&':
          data.replace(i, 1, "&amp;");
          break;

        case '<':
          data.replace(i, 1, "&lt;");
          break;

        case '>':
          data.replace(i, 1, "&gt;");
          break;
      }
    }
  }

  AGENT_LIB_API std::string addNamespace(const std::string aPath, const std::string aPrefix);

  // Ends with
  inline bool ends_with(const std::string &value, const std::string_view &ending)
  {
    if (ending.size() > value.size())
      return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
  }

  inline std::string ltrim(std::string s)
  {
    boost::algorithm::trim_left(s);
    return s;
  }

  // trim from end (in place)
  static inline std::string rtrim(std::string s)
  {
    boost::algorithm::trim_right(s);
    return s;
  }

  // trim from both ends (in place)
  inline std::string trim(std::string s)
  {
    boost::algorithm::trim(s);
    return s;
  }

  inline bool starts_with(const std::string &value, const std::string_view &beginning)
  {
    if (beginning.size() > value.size())
      return false;
    return std::equal(beginning.begin(), beginning.end(), value.begin());
  }

  /// @public
  /// @brief Case insensitive equals
  /// @param a first string
  /// @param b second string
  /// @return `true` if equal
  inline bool iequals(const std::string &a, const std::string_view &b)
  {
    if (a.size() != b.size())
      return false;

    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char a, char b) {
             return tolower(a) == tolower(b);
           });
  }

  using Attributes = std::map<std::string, std::string>;

  /// @brief overloaded pattern for variant visitors using list of lambdas
  /// @tparam ...Ts list of lambda classes
  template <class... Ts>
  struct overloaded : Ts...
  {
    using Ts::operator()...;
  };
  template <class... Ts>
  overloaded(Ts...) -> overloaded<Ts...>;

  /// @brief Reverse an iterable
  /// @tparam T The iterable type
  template <typename T>
  class reverse
  {
  private:
    T &m_iterable;

  public:
    explicit reverse(T &iterable) : m_iterable(iterable) {}
    auto begin() const { return std::rbegin(m_iterable); }
    auto end() const { return std::rend(m_iterable); }
  };

  using SequenceNumber_t = uint64_t;
  using FilterSet = std::set<std::string>;
  using FilterSetOpt = std::optional<FilterSet>;
  using Milliseconds = std::chrono::milliseconds;
  using Microseconds = std::chrono::microseconds;
  using Seconds = std::chrono::seconds;
  using Timestamp = std::chrono::time_point<std::chrono::system_clock>;
  using StringList = std::list<std::string>;
  using ConfigOption = std::variant<std::monostate, bool, int, std::string, double, Seconds,
                                    Milliseconds, StringList>;
  using ConfigOptions = std::map<std::string, ConfigOption>;
  template <typename T>
  inline const std::optional<T> GetOption(const ConfigOptions &options, const std::string &name)
  {
    auto v = options.find(name);
    if (v != options.end())
      return std::get<T>(v->second);
    else
      return std::nullopt;
  }

  inline bool IsOptionSet(const ConfigOptions &options, const std::string &name)
  {
    auto v = options.find(name);
    if (v != options.end())
      return std::get<bool>(v->second);
    else
      return false;
  }

  inline bool HasOption(const ConfigOptions &options, const std::string &name)
  {
    auto v = options.find(name);
    return v != options.end();
  }

  inline auto ConvertOption(const std::string &s, const ConfigOption &def)
  {
    ConfigOption option;
    visit(overloaded {[&option, &s](const std::string &) {
                        if (s.empty())
                          option = std::monostate();
                        else
                          option = s;
                      },
                      [&option, &s](const int &) { option = stoi(s); },
                      [&option, &s](const Milliseconds &) { option = Milliseconds {stoi(s)}; },
                      [&option, &s](const Seconds &) { option = Seconds {stoi(s)}; },
                      [&option, &s](const double &) { option = stod(s); },
                      [&option, &s](const bool &) { option = s == "yes" || s == "true"; },
                      [](const auto &) {}},
          def);
    return option;
  }

  inline int64_t ConvertFileSize(const ConfigOptions &options, const std::string &name,
                                 int64_t size = 0)
  {
    using namespace std;
    using boost::regex;
    using boost::smatch;

    auto value = GetOption<string>(options, name);
    if (value)
    {
      static const regex pat("([0-9]+)([GgMmKkBb]*)");
      smatch match;
      string v = *value;
      if (regex_match(v, match, pat))
      {
        size = boost::lexical_cast<int64_t>(match[1]);
        if (match[2].matched)
        {
          switch (match[2].str()[0])
          {
            case 'G':
            case 'g':
              size *= 1024;

            case 'M':
            case 'm':
              size *= 1024;

            case 'K':
            case 'k':
              size *= 1024;
          }
        }
      }
      else
      {
        std::stringstream msg;
        msg << "Invalid value for " << name << ": " << *value << endl;
        throw std::runtime_error(msg.str());
      }
    }

    return size;
  }

  inline void AddOptions(const boost::property_tree::ptree &tree, ConfigOptions &options,
                         const ConfigOptions &entries)
  {
    for (auto &e : entries)
    {
      auto val = tree.get_optional<std::string>(e.first);
      if (val)
      {
        auto v = ConvertOption(*val, e.second);
        if (v.index() != 0)
          options.insert_or_assign(e.first, v);
      }
    }
  }

  inline void AddDefaultedOptions(const boost::property_tree::ptree &tree, ConfigOptions &options,
                                  const ConfigOptions &entries)
  {
    for (auto &e : entries)
    {
      auto val = tree.get_optional<std::string>(e.first);
      if (val)
      {
        auto v = ConvertOption(*val, e.second);
        if (v.index() != 0)
          options.insert_or_assign(e.first, v);
      }
      else if (options.find(e.first) == options.end())
        options.insert_or_assign(e.first, e.second);
    }
  }

  inline void MergeOptions(ConfigOptions &options, const ConfigOptions &entries)
  {
    for (auto &e : entries)
    {
      options.insert_or_assign(e.first, e.second);
    }
  }

  inline void GetOptions(const boost::property_tree::ptree &tree, ConfigOptions &options,
                         const ConfigOptions &entries)
  {
    for (auto &e : entries)
    {
      if (!std::holds_alternative<std::string>(e.second) ||
          !std::get<std::string>(e.second).empty())
      {
        options.emplace(e.first, e.second);
      }
    }
    AddOptions(tree, options, entries);
  }

  inline std::string format(const Timestamp &ts)
  {
    using namespace std;
    string time = date::format("%FT%T", date::floor<Microseconds>(ts));
    auto pos = time.find_last_not_of("0");
    if (pos != string::npos)
    {
      if (time[pos] != '.')
        pos++;
      time.erase(pos);
    }
    time.append("Z");
    return time;
  }

  inline void capitalize(std::string::iterator start, std::string::iterator end)
  {
    using namespace std;

    // Exceptions to the rule
    const static std::unordered_map<std::string, std::string> exceptions = {
        {"AC", "AC"}, {"DC", "DC"},   {"PH", "PH"},
        {"IP", "IP"}, {"URI", "URI"}, {"MTCONNECT", "MTConnect"}};

    const auto &w = exceptions.find(std::string(start, end));
    if (w != exceptions.end())
    {
      copy(w->second.begin(), w->second.end(), start);
    }
    else
    {
      *start = ::toupper(*start);
      start++;
      transform(start, end, start, ::tolower);
    }
  }

  inline std::string pascalize(const std::string &type, std::optional<std::string> &prefix)
  {
    using namespace std;
    if (type.empty())
      return "";

    string camel;
    auto colon = type.find(':');

    if (colon != string::npos)
    {
      prefix = type.substr(0ul, colon);
      camel = type.substr(colon + 1ul);
    }
    else
      camel = type;

    auto start = camel.begin();
    decltype(start) end;

    bool done;
    do
    {
      end = find(start, camel.end(), '_');
      capitalize(start, end);
      done = end == camel.end();
      if (!done)
      {
        camel.erase(end);
        start = end;
      }
    } while (!done);

    return camel;
  }

  inline Timestamp parseTimestamp(const std::string &timestamp)
  {
    using namespace date;
    using namespace std::chrono;
    using namespace std::chrono_literals;
    using namespace date::literals;

    Timestamp ts;
    std::istringstream in(timestamp);
    in >> std::setw(6) >> parse("%FT%T", ts);
    if (!in.good())
    {
      ts = std::chrono::system_clock::now();
    }
    return ts;
  }

#define SCHEMA_VERSION(major, minor) (major * 100 + minor)

  inline std::string StrDefaultSchemaVersion()
  {
    return std::to_string(AGENT_VERSION_MAJOR) + "." + std::to_string(AGENT_VERSION_MINOR);
  }

  inline constexpr int32_t IntDefaultSchemaVersion()
  {
    return SCHEMA_VERSION(AGENT_VERSION_MAJOR, AGENT_VERSION_MINOR);
  }

  inline int32_t IntSchemaVersion(const std::string &s)
  {
    int major, minor;
    char c;
    std::stringstream vstr(s);
    vstr >> major >> c >> minor;
    return SCHEMA_VERSION(major, minor);
  }
}  // namespace mtconnect
