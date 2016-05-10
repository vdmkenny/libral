#include <libral/libral.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/nowide/args.hpp>
#include <leatherman/logging/logging.hpp>

// boost includes are not always warning-clean. Disable warnings that
// cause problems before including the headers, then re-enable the warnings.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#include <boost/program_options.hpp>
#pragma GCC diagnostic pop

using namespace std;
using namespace leatherman::logging;
namespace po = boost::program_options;

void help(po::options_description& desc)
{
  boost::nowide::cout <<
    "Synopsis\n"
    "========\n"
    "\n"
    "Uses the Puppet RAL to directly interact with the system.\n"
    "\n"
    "Usage\n"
    "=====\n"
    "\n"
    /* Not implemented:
       [-d|--debug] [-v|--verbose] [-e|--edit] [-p|--param parameter] [-t|--types] [-y|--to_yaml]
    */
    "  ralsh [-h|--help] type [name] [attribute=value ...]\n"
    "\n"
    "Options\n"
    "=======\n\n" << desc <<
    "\nDescription\n"
    "===========\n"
    "This command provides simple facilities for converting current system state\n"
    "into Puppet code, along with some ability to modify the current state using\n"
    "Puppet's RAL."
    "\n" << endl;
}

int main(int argc, char **argv) {
  try {
    // Fix args on Windows to be UTF-8
    boost::nowide::args arg_utf8(argc, argv);

    // Setup logging
    setup_logging(boost::nowide::cerr);

    po::options_description command_line_options("");
    command_line_options.add_options()
      ("help,h", "produce help message")
      ("log-level,l", po::value<log_level>()->default_value(log_level::warning, "warn"), "Set logging level.\nSupported levels are: none, trace, debug, info, warn, error, and fatal.")
      ("version,v", "print the version and exit");

    /* Positional options */
    command_line_options.add_options()
      ("type", po::value<std::string>())
      ("name", po::value<std::string>())
      ("attr-value", po::value<std::vector<std::string>>());


    po::positional_options_description positional_options;
    positional_options.add("type", 1);
    positional_options.add("name", 1);
    positional_options.add("attr-value", -1);

    po::variables_map vm;

    try {
      po::store(po::command_line_parser(argc, argv).
                options(command_line_options).
                positional(positional_options).run(), vm);

      if (vm.count("help")) {
        help(command_line_options);
        return EXIT_SUCCESS;
      }

      po::notify(vm);
    } catch (exception& ex) {
      colorize(boost::nowide::cerr, log_level::error);
      boost::nowide::cerr << "error: " << ex.what() << "\n" << endl;
      colorize(boost::nowide::cerr);
      boost::nowide::cerr << "Try 'ralsh -h' for more information." << endl;
      return EXIT_FAILURE;
    }

    // Get the logging level
    auto lvl = vm["log-level"].as<log_level>();
    set_level(lvl);

    if (vm.count("version")) {
      boost::nowide::cout << libral::version() << endl;
      return EXIT_SUCCESS;
    }

    if (vm.count("type")) {
      boost::nowide::cout << "Type: " << vm["type"].as<std::string>() << endl;
      if (vm.count("name")) {
        boost::nowide::cout << "Name: " << vm["name"].as<std::string>() << endl;
        if (vm.count("attr-value")) {
          auto av = vm["attr-value"].as<std::vector<std::string>>();
          for (auto arg = av.begin(); arg != av.end(); arg++) {
            auto found = arg->find("=");
            if (found != string::npos) {
              auto attr = arg->substr(0, found);
              auto value = arg->substr(found+1);
              boost::nowide::cout << attr << " : " << value << endl;
            }
          }
        } else {
          boost::nowide::cout << "Dump resource" << endl;
        }
      } else {
        boost::nowide:: cout << "List instances" << endl;
      }
    } else {
      boost::nowide:: cout << "List types" << endl;
    }
  } catch (exception& ex) {
    colorize(boost::nowide::cerr, log_level::fatal);
    boost::nowide::cerr << "unhandled exception: " << ex.what() << "\n" << endl;
    colorize(boost::nowide::cerr);
    return EXIT_FAILURE;
  }

  return error_has_been_logged() ? EXIT_FAILURE : EXIT_SUCCESS;
}
