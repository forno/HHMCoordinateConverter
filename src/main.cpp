#include <cstddef>
#include <filesystem>
#include <iostream>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/tokenizer.hpp>

#include <Eigen/Dense>

constexpr std::size_t marker_count {29};

struct index_holder
{
  std::size_t lasis;
  std::size_t rasis;
  std::size_t vsacral;
};

index_holder read_indexies(std::filesystem::path path);

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cerr << "Usage: " << *argv << " IndexiesFilePath\n";
    return 1;
  }

  try {
    const auto indexies {read_indexies(argv[1])};
    for (std::string line; std::getline(std::cin, line);) {
      boost::tokenizer<boost::escaped_list_separator<char>> tokens(line);
      Eigen::Matrix<float, marker_count, 3> values;
      std::size_t current_index {0};
      for (auto v : tokens) {
        values(current_index / 3, current_index % 3) = std::stof(v);
        ++current_index;
      }
      std::cout << values;
    }
  } catch(boost::wrapexcept<boost::property_tree::ptree_bad_path>& e) {
    std::cerr << "Invalid indexies format: " << e.what() << '\n';
    return 1;
  } catch(boost::wrapexcept<boost::property_tree::ini_parser::ini_parser_error>& e) {
    std::cerr << "Invalid indexies file: " << e.what() << '\n';
    return 1;
  }

  return 0;
}

index_holder read_indexies(std::filesystem::path path)
{
  using namespace boost::property_tree;
  ptree pt;
  read_ini(path.native(), pt);
  return index_holder {pt.get<std::size_t>("Indexies.lasis"), pt.get<std::size_t>("Indexies.rasis"), pt.get<std::size_t>("Indexies.vsacral")};
}
