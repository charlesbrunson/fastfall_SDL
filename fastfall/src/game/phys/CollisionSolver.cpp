#include "fastfall/game/phys/CollisionSolver.hpp"

#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"

//#include <ranges>

#include "nlohmann/json.hpp"

namespace ff 
{

NLOHMANN_JSON_SERIALIZE_ENUM(ContactType, {
	{ContactType::NO_SOLUTION,		"no solution"},
	{ContactType::SINGLE,			"single"},
	{ContactType::WEDGE,			"wedge"},
	{ContactType::CRUSH_HORIZONTAL,	"horizontal crush"},
	{ContactType::CRUSH_VERTICAL,	"vertical crush"},
})

nlohmann::ordered_json to_json(const Contact* contact)
{
	const auto* arb = contact->arbiter;
	return {
		{"contact",			fmt::format("{}", fmt::ptr(contact)) },
		{"hasContact",		contact->hasContact},
		{"separation",		contact->separation},
		{"ortho_n",			fmt::format("{}", contact->ortho_n)},
		{"collider_n",		fmt::format("{}", contact->collider_n)},
		{"hasImpactTime",	contact->hasImpactTime},
		{"impactTime",		contact->impactTime},
		{"velocity",		fmt::format("{}", contact->velocity)},
		{"region",			arb && arb->region   ? arb->region->get_ID().value : 0u},
		{"quad",			arb && arb->collider ? arb->collider->getID()      : 0u}
	};
}


CollisionSolver::CollisionSolver(Collidable* _collidable) :
	collidable(_collidable),
	allCollisionCount({ 0u, 0u, 0u, 0u }),
	initialCollisionCount({ 0u, 0u, 0u, 0u }),
	appliedCollisionCount({ 0u, 0u, 0u, 0u }),
	applyCounter(0u)
{
}


void CollisionSolver::updateContact(Contact* contact)
{
	if (contact->arbiter) {
		contact->arbiter->update(0.0);
	}
}

void CollisionSolver::updateStack(std::deque<Contact*>& stack) {
	for (auto c : stack) {
		updateContact(c);
	}
}

void CollisionSolver::compareAll() {

	if (json_dump)
	{
		size_t ndx = 0;
		for (auto& c : contacts) {
			(*json_dump)["precompare"][ndx] = to_json(c);
			ndx++;
		}

		(*json_dump)["compare"];
	}

	size_t cmp_count = 0;
	for (size_t i = 0; i < contacts.size() - 1; i++) {
		for (size_t j = i + 1; j < contacts.size(); ) {

			CompResult result = compare(contacts.at(i), contacts.at(j));

			if (json_dump)
			{
				(*json_dump)["compare"][cmp_count] = {
					{"first",  fmt::format("{}", fmt::ptr(contacts.at(i))) },
					{"second", fmt::format("{}", fmt::ptr(contacts.at(j))) },
					{"result", {
							{"first",  (result.discardFirst ? "discard" : "keep")},
							{"second", (result.discardSecond ? "discard" : "keep")}
						}
					}
				};
			}

			if (result.discardFirst) {
				discard.push_back(std::make_pair(*(contacts.begin() + i), result));
				contacts.erase(contacts.begin() + i);
				i--;
				break;
			}
			else if (result.discardSecond) {
				discard.push_back(std::make_pair(*(contacts.begin() + j), result));
				contacts.erase(contacts.begin() + j);
			}
			else {
				j++;
			}
			cmp_count++;
		}
	}

	if (json_dump)
	{
		size_t ndx = 0;
		for (auto c : contacts) {
			//auto& contact = *arb->getContactPtr();

			(*json_dump)["postcompare"][ndx] = to_json(c);
			ndx++;
		}

		(*json_dump)["apply"];
	}
}

void CollisionSolver::initStacks()
{

	for (auto* contact : contacts) {
		//const Contact* contact = arb->getContactPtr();

		auto dir = direction::from_vector(contact->ortho_n);
		if (!dir.has_value()) {
			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard_nodir"] = fmt::format("{}", fmt::ptr(contact));
				applyCounter++;
			}
			discard.push_back(std::make_pair(contact, CompResult{}));
			continue;
		}

		allCollisionCount[dir.value()]++;
		if (contact->hasContact)
			initialCollisionCount[dir.value()]++;

		switch (dir.value()) {
		case Cardinal::E:
			east.push_back(contact);
			break;
		case Cardinal::W:
			west.push_back(contact);
			break;
		case Cardinal::N:
			(contact->isTransposable() ? north_alt : north).push_back(contact);
			break;
		case Cardinal::S:
			(contact->isTransposable() ? south_alt : south).push_back(contact);
			break;
		}
	}
}


std::optional<Contact> CollisionSolver::detectWedge(const Contact* north, const Contact* south)
{

	float nSep = north->separation;
	float sSep = south->separation;
	float crush = nSep + sSep;
	bool any_has_contact = north->hasContact || south->hasContact;

	Rectf colBox = collidable->getBox();

	std::optional<Contact> contact;

	if (crush >= 0.f && any_has_contact
		&& north->ortho_n == -south->ortho_n)
	{

		Vec2f sum = north->collider_n + south->collider_n;

		if (abs(sum.x) < 1e-5
			&& abs(sum.y) < 1e-5)
		{
		}
		else {
			Angle floorAng = math::angle(north->collider.surface);
			Angle ceilAng = math::angle(south->collider.surface);

			Linef floorLine = north->collider.surface;
			Linef ceilLine = math::shift(south->collider.surface, Vec2f{ 0.f, colBox.height });

			Vec2f floorVel = north->velocity;
			Vec2f ceilVel = south->velocity;

			Vec2f intersect = math::intersection(floorLine, ceilLine);

			if (std::isnan(intersect.x) || std::isnan(intersect.y)) {
				LOG_WARN("bad intersection");
			}
			else if (intersect.x != collidable->getPosition().x) 
			{
				Vec2f pos = collidable->getPosition();

				Vec2f diff = Vec2f(intersect.x, 0.f) - Vec2f(pos.x, 0.f);
				float side = (north->collider_n.x + south->collider_n.x < 0.f ? -1.f : 1.f);

				contact = Contact{};
				contact->hasContact = true;
				contact->separation = abs(diff.x);
				contact->collider_n = Vec2f{ side, 0.f };
				contact->ortho_n = contact->collider_n;
				contact->position = Vec2f{ pos.x, math::rect_mid(colBox).y };

				contact->velocity = intersect - math::intersection(
					math::shift(floorLine, -floorVel),
					math::shift(ceilLine, -ceilVel)
				);
			}
		}
	}
	return contact;
}

void CollisionSolver::detectWedges() {
	for (auto& north_arb : north)
	{
		for (auto& south_arb : south)
		{
			if (auto opt_contact = detectWedge(north_arb, south_arb))
			{
				auto dir = direction::from_vector(opt_contact->ortho_n);
				if (dir && *dir == Cardinal::E)
				{
					created_contacts.push_back(*opt_contact);
					east.push_back(&created_contacts.back());
				}
				else if (dir && *dir == Cardinal::W) 
				{
					created_contacts.push_back(*opt_contact);
					west.push_back(&created_contacts.back());
				}
			}
		}
	}
}

void CollisionSolver::solve(nlohmann::ordered_json* dump_ptr) 
{
	json_dump = dump_ptr;

	if (contacts.empty())
		return;

	// do contact-to-contact comparisons 
	// try to discard some redundant ones early
	compareAll();

	// organize contacts into north/east/south/west
	initStacks();

	// opportunistically determine if we can solve steeper slopes on the X axis instead of Y
	if (canApplyAlt(north_alt, south_alt)) {
		applyAltStack(north_alt, north);
		applyAltStack(south_alt, south);
	}
	else {
		// else bail out
		north.insert(north.end(), north_alt.begin(), north_alt.end());
		south.insert(south.end(), south_alt.begin(), south_alt.end());
		north_alt.clear();
		south_alt.clear();
	}

	// detect any wedges
	// a wedge is any pair of north/south contacts that would force the collision box east/west instead
	detectWedges();

	// solve X axis
	solveX();

	// update north/south contacts for our new X-pos
	updateStack(north);
	updateStack(south);

	// solve Y axis
	solveY();
}

void CollisionSolver::solveX() {

	std::sort(east.begin(), east.end(), ContactCompare);
	std::sort(west.begin(), west.end(), ContactCompare);

	if (json_dump) {
		(*json_dump)["apply"][applyCounter]["x_stacks"];
		size_t jndx = 0;
		for (auto& e : east) {
			(*json_dump)["apply"][applyCounter]["x_stacks"]["east"][jndx] = fmt::format("{}", fmt::ptr(e));
			jndx++;
		}
		jndx = 0;
		for (auto& w : west) {
			(*json_dump)["apply"][applyCounter]["x_stacks"]["west"][jndx] = fmt::format("{}", fmt::ptr(w));
			jndx++;
		}
		applyCounter++;
	}

	while (!east.empty() && !west.empty()) {
		auto eastArb = east.front();
		auto westArb = west.front();

		if (json_dump) {
			(*json_dump)["apply"][applyCounter]["picking_from"] = {
				{ "east", fmt::format("{}", fmt::ptr(eastArb)) },
				{ "west", fmt::format("{}", fmt::ptr(westArb)) }
			};
		}

		auto r = pickH(eastArb, westArb);

		if (r.contact) {
			apply(*r.contact, r.contactType);

			if (r.contactType == ContactType::CRUSH_HORIZONTAL)
				return;

			if (!r.discardFirst) updateContact(eastArb);
			if (!r.discardSecond) updateContact(westArb);
		}

		if (r.discardFirst && !r.discardSecond) {

			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard_x"] = fmt::format("{}", fmt::ptr(east.front()));
				applyCounter++;
			}

			east.pop_front();
			applyFirst(west);
			updateStack(east);
		}
		else if (r.discardSecond && !r.discardFirst) {

			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard_x"] = fmt::format("{}", fmt::ptr(west.front()));
				applyCounter++;
			}

			west.pop_front();
			applyFirst(east);
			updateStack(west);
		}
		else if (r.discardFirst && r.discardSecond) {

			if (json_dump) {
				(*json_dump)["apply"][applyCounter] = {
					{ "discard_x", {
							fmt::format("{}", fmt::ptr(north.front())),
							fmt::format("{}", fmt::ptr(south.front()))
						}
					}
				};
			}

			east.pop_front();
			west.pop_front();
		}
		else {
			if (eastArb->separation < westArb->separation) {
				applyFirst(east);
				updateStack(west);
			}
			else {
				applyFirst(west);
				updateStack(east);
			}
		}
	}

	applyStack(east);
	applyStack(west);
}

void CollisionSolver::solveY() {

	std::sort(north.begin(), north.begin(), ContactCompare);
	std::sort(south.begin(), south.begin(), ContactCompare);
	
	if (json_dump) {
		(*json_dump)["apply"][applyCounter]["y_stacks"];
		size_t jndx = 0;
		for (auto& n : north) {
			(*json_dump)["apply"][applyCounter]["y_stacks"]["north"][jndx] = fmt::format("{}", fmt::ptr(n));
			jndx++;
		}
		jndx = 0;
		for (auto& s : south) {
			(*json_dump)["apply"][applyCounter]["y_stacks"]["south"][jndx] = fmt::format("{}", fmt::ptr(s));
			jndx++;
		}
		applyCounter++;
	}

	while (!north.empty() && !south.empty()) {
		auto northArb = north.front();
		auto southArb = south.front();

		if (json_dump) {
			(*json_dump)["apply"][applyCounter]["picking_from"] = {
				{ "north", fmt::format("{}", fmt::ptr(northArb)) },
				{ "south", fmt::format("{}", fmt::ptr(southArb)) }
			};
		}

		auto r = pickV(northArb, southArb);

		if (r.contact) {
			apply(*r.contact, r.contactType);

			if (r.contactType == ContactType::CRUSH_VERTICAL)
				return;

			if (!r.discardFirst)  updateContact(northArb);
			if (!r.discardSecond) updateContact(southArb);

			if (!northArb->hasContact
				&& !southArb->hasContact) 
			{
				if (json_dump) {

					(*json_dump)["apply"][applyCounter] = {
						{ "discard_y_contact", {
								fmt::format("{}", fmt::ptr(north.front())),
								fmt::format("{}", fmt::ptr(south.front()))
							}
						}
					};
					applyCounter++;
				}
				north.pop_front();
				south.pop_front();
				continue;
			}

			if (!northArb->hasContact) {
				if (json_dump) {
					(*json_dump)["apply"][applyCounter]["discard_y_contact"] = fmt::format("{}", fmt::ptr(north.front()));
					applyCounter++;
				}
				north.pop_front();
				continue;
			}
			if (!southArb->hasContact) {
				if (json_dump) {
					(*json_dump)["apply"][applyCounter]["discard_y_contact"] = fmt::format("{}", fmt::ptr(south.front()));
					applyCounter++;
				}
				south.pop_front();
				continue;
			}

		}

		if (r.discardFirst && !r.discardSecond) {

			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard_y_nocontact"] = fmt::format("{}", fmt::ptr(north.front()));
				applyCounter++;
			}

			north.pop_front();
			applyFirst(south);
			updateStack(north);
		}
		else if (r.discardSecond && !r.discardFirst) {
			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard_y_nocontact"] = fmt::format("{}", fmt::ptr(south.front()));
				applyCounter++;
			}

			south.pop_front();
			applyFirst(north);
			updateStack(south);
		}
		else if (r.discardFirst && r.discardSecond) {

			if (json_dump) {
				(*json_dump)["apply"][applyCounter] = {
					{ "discard_y_nocontact", {
							fmt::format("{}", fmt::ptr(north.front())),
							fmt::format("{}", fmt::ptr(south.front()))
						}
					}
				};
			}

			north.pop_front();
			south.pop_front();
		}
		else {
			if (northArb->separation < southArb->separation) {
				applyFirst(north);
				updateStack(south);
			}
			else {
				applyFirst(south);
				updateStack(north);
			}
		}
	}

	applyStack(north);
	applyStack(south);
}


// ----------------------------------------------------------------------------

void CollisionSolver::applyStack(std::deque<Contact*>& stack) {
	while (!stack.empty()) {
		applyFirst(stack);
	}
}

void CollisionSolver::applyFirst(std::deque<Contact*>& stack) {
	if (stack.empty())
		return;

	// if there are multiple arbiters with the same sep, prefer the one closest to the collidable's center
	std::deque<Contact*>::iterator pick = stack.begin();
	if (stack.size() > 1) 
	{
		float sep = (*pick)->separation;

		for (auto it = stack.begin() + 1; it != stack.end(); it++) 
		{
			if ((*it)->separation != sep) {
				break;
			}
			else {
				const Contact* c1 = *pick;
				const Contact* c2 = *it;
				Vec2f mid = math::rect_mid(collidable->getBox());

				if (math::is_horizontal((*pick)->ortho_n)) {
					if (abs(c2->position.y - mid.y) < abs(c1->position.y - mid.y)) {
						pick = it;
						continue;
					}
				}
				else {
					if (abs(c2->position.x - mid.x) < abs(c1->position.x - mid.x)) {
						pick = it;
						continue;
					}
				}
			}
		}
	}
	apply(**pick);
	stack.erase(pick);

	updateStack(stack);
}

bool CollisionSolver::canApplyAlt(std::deque<Contact*>& north_alt, std::deque<Contact*>& south_alt) const
{

	/*
	bool allWest = east.empty() 
		&& std::all_of(north_alt.cbegin(), north_alt.cend(), [](const Arbiter* arb) {
			float nX = arb->getContactPtr()->collider_n.x;
			return (nX < 0.f) && (nX != 0.f);
		}) 
		&& std::all_of(south_alt.cbegin(), south_alt.cend(), [](const Arbiter* arb) {
			float nX = arb->getContactPtr()->collider_n.x;
			return (nX < 0.f) && (nX != 0.f);
		});
	bool allEast = west.empty() 
		&& std::all_of(north_alt.cbegin(), north_alt.cend(), [](const Arbiter* arb) {
			float nX = arb->getContactPtr()->collider_n.x;
			return (nX > 0.f) && (nX != 0.f);
		}) 
		&& std::all_of(south_alt.cbegin(), south_alt.cend(), [](const Arbiter* arb) {
			float nX = arb->getContactPtr()->collider_n.x;
			return (nX > 0.f) && (nX != 0.f);
		});

	return (!north_alt.empty() || !south_alt.empty()) && (allWest || allEast);
	*/

	auto isArbEast = [](const Contact* c) {
		return c->collider_n.x > 0.f;
	};

	auto isArbWest = [](const Contact* c) {
		return c->collider_n.x < 0.f;
	};

	bool allNorthAltWest = std::all_of(north_alt.cbegin(), north_alt.cend(), isArbWest);
	bool allSouthAltWest = std::all_of(south_alt.cbegin(), south_alt.cend(), isArbWest);

	bool allNorthAltEast = std::all_of(north_alt.cbegin(), north_alt.cend(), isArbEast);
	bool allSouthAltEast = std::all_of(south_alt.cbegin(), south_alt.cend(), isArbEast);

	bool allWest = east.empty() && allNorthAltWest && allSouthAltWest;
	bool allEast = west.empty() && allNorthAltEast && allSouthAltEast;

	return (allWest || allEast);
}

void CollisionSolver::applyAltStack(std::deque<Contact*>& altList, std::deque<Contact*>& backupList) {

	while (!altList.empty()) 
	{
		const Contact* c = altList.front();

		bool updateRemaining = false;

		if (c->hasContact) {
			//assert(abs(c->collider_n.x) > 0.f && abs(c->collider_n.x) < 1.f);

			Vec2f alt_ortho_normal = (c->collider_n.x < 0.f ? Vec2f(-1.f, 0.f) : Vec2f(1.f, 0.f));
			float alt_separation = abs((c->collider_n.y * c->separation) / c->collider_n.x);

			CompResult r;
			r.contactType = ContactType::SINGLE;
			r.contact = *c;
			r.contact->ortho_n = alt_ortho_normal;
			r.contact->separation = alt_separation;
			apply(*r.contact, r.contactType);
			updateRemaining = true;
		}

		altList.pop_front();

		if (updateRemaining) {
			updateStack(altList);
		}
	}
}

// ----------------------------------------------------------------------------

void CollisionSolver::apply(const Contact& contact, ContactType type)  {


	if (contact.hasContact) {
		appliedCollisionCount[direction::from_vector(contact.ortho_n).value()]++;

		if (json_dump) {
			(*json_dump)["apply"][applyCounter] = to_json(&contact);
			(*json_dump)["apply"][applyCounter]["type"] = type;
		}

		collidable->applyContact(contact, type);

		AppliedContact applied;
		applied.contact = contact;
		applied.type = type;

		if (contact.arbiter != nullptr) {
			contact.arbiter->setApplied();
			contact.arbiter->update(0.0);
		}

		frame.push_back(applied);
		applyCounter++;
	}
	else {
		if (json_dump) {
			(*json_dump)["apply"][applyCounter]["discard_apply"] = fmt::format("{}", fmt::ptr(&contact));
			applyCounter++;
		}
	}
}

// ----------------------------------------------------------------------------


CollisionSolver::CompResult CollisionSolver::compare(const Contact* lhs, const Contact* rhs) {
	CompResult comp;

	if (lhs == rhs)
		return comp;

	const Contact& lhsContact = *lhs;
	const Contact& rhsContact = *rhs;

	comp.discardFirst  = lhs->arbiter ? !lhs->arbiter->getCollision()->tileValid() : false;
	comp.discardSecond = rhs->arbiter ? !rhs->arbiter->getCollision()->tileValid() : false;

	// ghost check
	if (!comp.discardFirst && !comp.discardSecond) 
	{
		Ghost g1 = isGhostEdge(rhsContact, lhsContact);
		Ghost g2 = isGhostEdge(lhsContact, rhsContact);

		bool g1_isGhost = (g1 != Ghost::NO_GHOST);
		bool g2_isGhost = (g2 != Ghost::NO_GHOST);

		if (g2_isGhost || g1_isGhost) {
			if (g2_isGhost && g1_isGhost) {
				// pick one
				if (g1 == g2) {
					// double ghost, pick the one with least separation
					g1_isGhost = !ContactCompare(lhs, rhs);
				}
				else {
					g1_isGhost = g2 < g1;
				}
				g2_isGhost = !g1_isGhost;
			}

			comp.discardFirst = g1_isGhost;
			comp.discardSecond = g2_isGhost;
		}
	}	
	return comp;
}

CollisionSolver::Ghost CollisionSolver::isGhostEdge(const Contact& basis, const Contact& candidate) noexcept {


	if (!basis.isResolvable())
		return CollisionSolver::Ghost::NO_GHOST;

	bool isOneWay = (!basis.hasContact) && (basis.separation > 0.f) && (basis.impactTime == -1.0);

	Linef basisLine = basis.collider.surface;
	Linef candLine = candidate.collider.surface;

	Vec2f basisNormal = math::vector(basis.collider.surface).lefthand().unit();

	float dotp1 = math::dot(basisNormal, candLine.p1 - basisLine.p2);
	float dotp2 = math::dot(basisNormal, candLine.p2 - basisLine.p1);

	bool is_ghost = false;

	// candidate is full *behind* basis surface
	bool opt1;
	if (   candLine.p1 == basisLine.p2
		|| candLine.p2 == basisLine.p1) 
	{
		// surfaces share a point
		opt1 = (dotp1 < 0.f && dotp2 < 0.f); // always false?
	}
	else {
		opt1 = (dotp1 <= 0.f && dotp2 < 0.f) || (dotp1 < 0.f && dotp2 <= 0.f);
	}

	// prefer selecting verticals as the ghost edge
	bool opt2 = (!math::is_vertical(basisLine) && math::is_vertical(candLine) && dotp1 <= 0.f && dotp2 <= 0.f); 

	// candidate is opposite of basis
	bool opt3 = (basisLine == Linef(candLine.p2, candLine.p1)); 

	bool candidateBehind = opt1 || opt2 || opt3;

	if (!isOneWay && candidateBehind) {
		return (opt1 ? CollisionSolver::Ghost::FULL_GHOST : CollisionSolver::Ghost::PARTIAL_GHOST);
	}
	else {
		return CollisionSolver::Ghost::NO_GHOST;
	}

}

// ----------------------------------------------------------------------------

CollisionSolver::CompResult CollisionSolver::pickH(const Contact* east, const Contact* west) {

	float eSep = east->separation;
	float wSep = west->separation;

	Rectf colBox = collidable->getBox();

	float crush = eSep + wSep;

	// diverging check
	if (eSep > colBox.width && wSep < colBox.width) {
		CompResult r;
		r.discardFirst = true;
		r.discardSecond = false;
		return r;
	}
	else if (eSep < colBox.width && wSep > colBox.width) {
		CompResult r;
		r.discardFirst = false;
		r.discardSecond = true;
		return r;
	}

	// crush
	if (crush > 0.f) {
		CompResult r;
		r.contactType = ContactType::CRUSH_HORIZONTAL;

		r.discardFirst = true;
		r.discardSecond = true;

		if (eSep > wSep) {
			r.contact = *east;
			r.contact->separation -= wSep;
			r.contact->separation /= 2.f;
		}
		else {
			r.contact = *west;
			r.contact->separation -= eSep;
			r.contact->separation /= 2.f;
		}
		return r;
	}

	if (!east->hasContact)
	{
		CompResult r;
		r.discardFirst = true;
		r.discardSecond = false;
		return r;
	}
	else if (!west->hasContact)
	{
		CompResult r;
		r.discardFirst = false;
		r.discardSecond = true;
		return r;
	}
	return CompResult{};
}

CollisionSolver::CompResult CollisionSolver::pickV(const Contact* north, const Contact* south) {

	//const Contact* nContact = north->getContactPtr();
	//const Contact* sContact = south->getContactPtr();

	float nSep = north->separation;
	float sSep = south->separation;

	Rectf colBox = collidable->getBox();

	float crush = nSep + sSep;

	bool any_has_contact = north->hasContact || south->hasContact;

	// diverging check
	if (crush > 0.f) {
		Vec2f nMid{ math::midpoint(north->collider.surface) };
		Vec2f sMid{ math::midpoint(south->collider.surface) };

		Vec2f nNormal = math::vector(north->collider.surface).lefthand().unit();
		bool nFacingAway = math::dot(sMid - nMid, nNormal) < 0;

		Vec2f sNormal = math::vector(south->collider.surface).lefthand().unit();
		bool sFacingAway = math::dot(nMid - sMid, sNormal) < 0;

		if ((nFacingAway && sFacingAway) || (crush > colBox.height)) {

			if (nSep < sSep) {
				CompResult r;
				r.discardFirst = false;
				r.discardSecond = true;
				return r;
			}
			else {
				CompResult r;
				r.discardFirst = true;
				r.discardSecond = false;
				return r;
			}
		}
	}

	// arbs are converging
	if (crush >= 0.f && any_has_contact
		&& north->ortho_n == -south->ortho_n) 
	{

		Vec2f sum = north->collider_n + south->collider_n;

		if (   abs(sum.x) < 1e-5
			&& abs(sum.y) < 1e-5)
		{

			// crush
			CompResult r;
			r.contactType = ContactType::CRUSH_VERTICAL;

			r.discardFirst = true;
			r.discardSecond = true;

			if (nSep > sSep) {
				r.contact = *north;
				r.contact->separation -= sSep;
				r.contact->separation /= 2.f;
			}
			else {
				r.contact = *south;
				r.contact->separation -= nSep;
				r.contact->separation /= 2.f;
			}
			return r;
		}
	}
	if (any_has_contact && !north->hasContact && south->hasContact)
	{
		CompResult r;
		r.discardFirst = true;
		r.discardSecond = false;
		return r;
	}
	else if (any_has_contact && north->hasContact && !south->hasContact)
	{
		CompResult r;
		r.discardFirst = false;
		r.discardSecond = true;
		return r;
	}

	CompResult r;
	r.discardFirst = false;
	r.discardSecond = false;
	return r;
}

}
