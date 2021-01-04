/* MIT License

Copyright (c) 2020 FORNO

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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

constexpr std::size_t marker_count{29};

struct index_holder
{
  std::size_t left_asis;
  std::size_t right_asis;
  std::size_t v_sacral;
};

index_holder read_indexies(std::filesystem::path path);
void convert4w2l(const index_holder&);
void convert4l2w(const index_holder&);

int main(int argc, char **argv)
{
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout << std::fixed;

  namespace po = boost::program_options;

  po::options_description description("Options");
  description.add_options()
    ("header,h", po::value<bool>()->default_value(true), "Dose the CSV has header?")
    ("config,c", po::value<std::string>(), "Config file path")
    ("l2w,l", po::value<bool>()->default_value(false), "Local to World flag")
    ("help,H", "Help")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, description), vm);
  po::notify(vm);

  if (argc < 2 || vm.count("help"))
  {
    std::cerr << "Usage: " << *argv << " --config config/Cooking [options] < cooking.csv\n";
    return 1;
  }

  try
  {
    const auto config_path = vm["config"].as<std::string>();
    const auto indexies{read_indexies(config_path)};
    if (vm["header"].as<bool>())
    {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    if (!vm["l2w"].as<bool>())
    {
      convert4w2l(indexies);
    }
    else
    {
      convert4l2w(indexies);
    }
  }
  catch (boost::wrapexcept<boost::property_tree::ptree_bad_path> &e)
  {
    std::cerr << "Invalid indexies format: " << e.what() << '\n';
    return 1;
  }
  catch (boost::wrapexcept<boost::property_tree::ini_parser::ini_parser_error> &e)
  {
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
  return index_holder{pt.get<std::size_t>("Indexies.LeftAsis"), pt.get<std::size_t>("Indexies.RightAsis"), pt.get<std::size_t>("Indexies.VSacral")};
}

void convert4w2l(const index_holder& indexies)
{
  for (std::string line; std::getline(std::cin, line);)
  {
    boost::tokenizer<boost::escaped_list_separator<char>> tokens(line);
    Eigen::Matrix<float, marker_count, 3, Eigen::RowMajor> values;
    {
      auto it = tokens.begin();
      for (std::size_t i{0}; i < marker_count * 3; ++i)
      {
        try
        {
          values(i) = std::stof(*it);
        }
        catch (std::invalid_argument &)
        {
          values(i) = std::numeric_limits<float>::quiet_NaN();
        }
        ++it;
      }
    }

    const Eigen::Vector3f center_asis = values.row(indexies.left_asis) / 2 + values.row(indexies.right_asis) / 2;
    for (std::size_t i{0}; i < marker_count; ++i)
    {
      values.row(i) -= center_asis;
    }

    const Eigen::Vector3f forward = -values.row(indexies.v_sacral);
    const Eigen::Vector3f forward_on_ground{forward(0), forward(1), 0};
    const auto w2l_rotation = Eigen::Quaternionf::FromTwoVectors(forward_on_ground, Eigen::Vector3f::UnitY());

    for (std::size_t i{0}; i < marker_count; ++i)
    {
      values.row(i) = w2l_rotation * values.row(i);
    }

    auto is_first{true};
    for (std::size_t i{0}; i < static_cast<std::size_t>(values.size()); ++i)
    {
      if (!std::exchange(is_first, false))
      {
        std::cout.put(',');
      }
      if (!isnan(values(i)))
      {
        std::cout << values(i);
      }
    }
    const auto l2w_translation{-center_asis};
    for (std::size_t i{0}; i < static_cast<std::size_t>(l2w_translation.size()); ++i)
    {
      std::cout << ',' << l2w_translation(i);
    }
    const auto l2w_rotation{w2l_rotation.inverse()};
    const auto &l2w_coeffs{l2w_rotation.coeffs()};
    for (std::size_t i{0}; i < static_cast<std::size_t>(l2w_coeffs.size()); ++i)
    {
      std::cout << ',' << l2w_coeffs(i);
    }
    std::cout.put('\n');
  }
}

void convert4l2w(const index_holder& indexies)
{
  for (std::string line; std::getline(std::cin, line);)
  {
    Eigen::Matrix<float, marker_count, 3, Eigen::RowMajor> values;
    Eigen::Vector3f translation;
    Eigen::Quaternionf rotation;
    {
      boost::tokenizer<boost::escaped_list_separator<char>> tokens(line);
      auto it = tokens.begin();
      for (std::size_t i{0}; i < static_cast<std::size_t>(values.size()); ++i)
      {
        try
        {
          values(i) = std::stof(*it);
        }
        catch (std::invalid_argument &)
        {
          values(i) = std::numeric_limits<float>::quiet_NaN();
        }
        ++it;
      }
      for (std::size_t i{0}; i < static_cast<std::size_t>(translation.size()); ++i)
      {
        try
        {
          translation(i) = std::stof(*it);
        }
        catch (std::invalid_argument &)
        {
          translation(i) = std::numeric_limits<float>::quiet_NaN();
        }
        ++it;
      }
      Eigen::Vector4f rotation_coeffs;
      for (std::size_t i{0}; i < static_cast<std::size_t>(rotation_coeffs.size()); ++i)
      {
        try
        {
          rotation_coeffs(i) = std::stof(*it);
        }
        catch (std::invalid_argument &)
        {
          rotation_coeffs(i) = std::numeric_limits<float>::quiet_NaN();
        }
        ++it;
      }
      rotation = Eigen::Quaternionf(rotation_coeffs);
    }
    for (std::size_t i{0}; i < marker_count; ++i)
    {
      values.row(i) = rotation * values.row(i) + translation;
    }
    auto is_first{true};
    for (std::size_t i{0}; i < static_cast<std::size_t>(values.size()); ++i)
    {
      if (!std::exchange(is_first, false))
      {
        std::cout.put(',');
      }
      if (!isnan(values(i)))
      {
        std::cout << values(i);
      }
    }
    std::cout.put('\n');
  }
}
