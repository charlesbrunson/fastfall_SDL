#include "application_list.hpp"

namespace ff {

application_list::application_list(std::unique_ptr<application> st) {
	root = std::move(st);
}

void application_list::update() {

	auto st = get_active_app();
	while (st && st->action != application_action::Continue) {
		switch (st->action) {
		case application_action::Next:
			if (st->next_app) {
				st->active = false;
				st->next_app->active = true;
			}
			st->action = application_action::Continue;
			break;

		case application_action::ExitNext:
			if (!st->next_app) {
				st->action = application_action::Continue;
				break;
			}
			st->next_app->prev_app = st->prev_app;
			if (st->prev_app) {
				st->prev_app->next_app.reset(st->next_app.release());
			}
			else {
				root.reset(st->next_app.release());
				st = root.get();
			}
            continue;

		case application_action::ExitPrev:
			if (st->prev_app) {
				// ask prev to free us
				st->prev_app->next_app.reset();
			}
			else {
				// we're root
				root.reset();
			}
			break;

		case application_action::CloseAll:
			clear();
			break;
        case application_action::Continue:
			break;
		}
		st = get_active_app();
	}
	if (st)
		st->active = true;
}

bool application_list::empty() const {
	return root.get() == nullptr;
};

void application_list::clear() {
	root.reset();
};

color application_list::get_clear_color() const {
	if (auto state = get_active_app()) {
		return state->get_clear_color();
	}
	return color::black;

}

application* application_list::get_active_app() const {
	application* st = root.get();
	while (st) {
		if (st->active)
			break;

		if (st->next_app.get()) {
			st = st->next_app.get();
		}
		else {
			break;
		}
	}
	if (st) st->active = true;
	return st;

};

}