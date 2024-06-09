
#include "application_runner.hpp"

namespace ff {

application_runner::application_runner(std::unique_ptr<application> init_app) :
	app_list{ std::move(init_app) }
{
}

}