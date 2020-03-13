#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/optional.hpp>

#include <Eigen/Dense>

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
  if (argc < 2) {
    std::cerr << "Usage: " << *argv << " IndexiesFilePath\n";
    return 1;
  }
  using namespace boost::property_tree;

  ptree pt;
  read_ini(argv[1], pt);

  if (auto value = pt.get_optional<int>("Indexies.FrontHead")) {
    std::cout << *value << std::endl;
  }
  return 0;
}
