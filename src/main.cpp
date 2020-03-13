#include <cmath>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <limits>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/tokenizer.hpp>

#include <Eigen/Dense>

constexpr std::size_t marker_count {29};

struct index_holder
{
  std::size_t left_asis;
  std::size_t right_asis;
  std::size_t v_sacral;
};

index_holder read_indexies(std::filesystem::path path);

int main(int argc, char** argv)
{
  namespace po = boost::program_options;

  po::options_description description("Options");
  description.add_options()
    ("header,h", po::value<bool>()->default_value(true), "Dose the CSV has header?")
    ("config,c", po::value<std::string>(), "Config file path")
    ("help,H", "Help")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, description), vm);
  po::notify(vm);

  if (argc < 2 || vm.count("help")) {
    std::cerr << "Usage: " << *argv << " --config config/Cooking < cooking.csv\n";
    return 1;
  }

  try {
    const auto indexies {read_indexies(vm["config"].as<std::string>())};
    if (vm["header"].as<bool>()) {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    for (std::string line; std::getline(std::cin, line);) {
      boost::tokenizer<boost::escaped_list_separator<char>> tokens(line);
      Eigen::Matrix<float, marker_count, 3> values;
      {
        auto it = tokens.begin();
        for (std::size_t i {0}; i < marker_count * 3; ++i) {
          try {
            values(i / 3, i % 3) = std::stof(*it);
          } catch (std::invalid_argument&) {
            values(i / 3, i % 3) = std::nan("");
          }
          ++it;
        }
      }
      const Eigen::Vector3f center_asis = values.row(indexies.left_asis) / 2 + values.row(indexies.right_asis) / 2;
      const Eigen::Vector3f l2r_vector = values.row(indexies.right_asis) - values.row(indexies.left_asis);
      std::cout << values;
    }
  } catch (boost::wrapexcept<boost::property_tree::ptree_bad_path>& e) {
    std::cerr << "Invalid indexies format: " << e.what() << '\n';
    return 1;
  } catch (boost::wrapexcept<boost::property_tree::ini_parser::ini_parser_error>& e) {
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
  return index_holder {pt.get<std::size_t>("Indexies.LeftAsis"), pt.get<std::size_t>("Indexies.RightAsis"), pt.get<std::size_t>("Indexies.VSacral")};
}
