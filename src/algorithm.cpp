#include "boost/actor/util/algorithm.hpp"

#include <sstream>

namespace boost {
namespace actor { namespace util {

std::vector<std::string> split(const std::string& str, char delim, bool keep_empties) {
    using namespace std;
    vector<string> result;
    stringstream strs{str};
    string tmp;
    while (getline(strs, tmp, delim)) {
        if (!tmp.empty() || keep_empties) result.push_back(std::move(tmp));
    }
    return result;
}

} } // namespace actor
} // namespace boost::util
