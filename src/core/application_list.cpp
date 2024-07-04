#include "ff/core/application_list.hpp"

namespace ff {

application_list::application_list()
{
}

application_list::application_list(std::unique_ptr<application>&& t_app)
: m_root(std::move(t_app))
{
}

void application_list::update() {

	auto st = get_active_app();
	while (st && st->m_action != application_action::Continue) {
		switch (st->m_action) {
		case application_action::Next:
			if (st->m_next_app) {
				st->m_active = false;
				st->m_next_app->m_active = true;
			}
			st->m_action = application_action::Continue;
			break;

		case application_action::ExitNext:
			if (!st->m_next_app) {
				st->m_action = application_action::Continue;
				break;
			}
			st->m_next_app->m_prev_app = st->m_prev_app;
			if (st->m_prev_app) {
				st->m_prev_app->m_next_app.reset(st->m_next_app.release());
			}
			else {
				m_root.reset(st->m_next_app.release());
				st = m_root.get();
			}
            continue;

		case application_action::ExitPrev:
			if (st->m_prev_app) {
				// ask prev to free us
				st->m_prev_app->m_next_app.reset();
			}
			else {
				// we're root
				m_root.reset();
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
		st->m_active = true;
}

bool application_list::empty() const {
	return m_root.get() == nullptr;
};

void application_list::clear() {
	m_root.reset();
};

application* application_list::get_active_app() const {
	application* st = m_root.get();
	while (st) {
		if (st->m_active)
			break;

		if (st->m_next_app.get()) {
			st = st->m_next_app.get();
		}
		else {
			break;
		}
	}
	if (st) {
        st->m_active = true;
    }
	return st;
}

}