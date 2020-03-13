#include <cmath>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <limits>
#include <utility>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/tokenizer.hpp>

#include <Eigen/Dense>
#include <Eigen/Geometry>

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
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout << std::fixed;

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
    const auto config_path = vm["config"].as<std::string>();
    const auto indexies {read_indexies(config_path)};
    if (vm["header"].as<bool>()) {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    for (std::string line; std::getline(std::cin, line);) {
      boost::tokenizer<boost::escaped_list_separator<char>> tokens(line);
      Eigen::Matrix<float, marker_count, 3, Eigen::RowMajor> values;
      {
        auto it = tokens.begin();
        for (std::size_t i {0}; i < marker_count * 3; ++i) {
          try {
            values(i) = std::stof(*it);
          } catch (std::invalid_argument&) {
            values(i) = std::numeric_limits<float>::quiet_NaN();
          }
          ++it;
        }
      }

      const Eigen::Vector3f center_asis = values.row(indexies.left_asis) / 2 + values.row(indexies.right_asis) / 2;
      for (std::size_t i {0}; i < marker_count; ++i) {
        values.row(i) -= center_asis;
      }

      const Eigen::Vector3f forward = -values.row(indexies.v_sacral);
      const auto w2l_rotation = Eigen::Quaternionf::FromTwoVectors(forward, Eigen::Vector3f{0, 0, 1.f});

      for (std::size_t i {0}; i < marker_count; ++i) {
        values.row(i) = w2l_rotation * values.row(i);
      }

      auto is_first {true};
      for (std::size_t i {0}; i < static_cast<std::size_t>(values.size()); ++i) {
        if (!std::exchange(is_first, false)) {
          std::cout.put(',');
        }
        if (!isnan(values(i))) {
          std::cout << values(i);
        }
      }
      std::cout.put('\n');
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
