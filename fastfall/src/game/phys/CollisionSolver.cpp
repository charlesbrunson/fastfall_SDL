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
		{"is_transposed",	contact->is_transposed},
		{"region",			arb && arb->region ? arb->region->get_ID().value : 0u},
		{"quad",			arb && arb->collider ? arb->collider->getID() : 0u}
	};
}

template<typename Container>
requires std::same_as<typename Container::value_type, Contact*>
nlohmann::ordered_json to_json(Container container)
{
	nlohmann::ordered_json json;
	for (const Contact* c : container)
	{
		json += fmt::format("{}", fmt::ptr(c));
	}
	return json;
}

void updateContact(Contact* contact)
{
	if (contact->arbiter) {
		contact->arbiter->update(0.0);
	}
}

void updateStack(std::deque<Contact*>& stack) {
	std::for_each(stack.begin(), stack.end(), updateContact);
}

bool CollisionSolver::applyThenUpdateStacks(std::deque<Contact*>& stack, std::deque<Contact*>& otherStack, bool which)
{
	bool applied = false;
	if (applied = applyFirst(which ? stack : otherStack)) {
		updateStack(stack);
		updateStack(otherStack);
	}
	return applied;
};

bool CollisionSolver::canApplyElseDiscard(bool discard, std::deque<Contact*>& stack)
{
	if (discard)
	{
		if (json_dump) {
			(*json_dump)["apply"] += {
				{ "discard_nocontact", fmt::format("{}", fmt::ptr(stack.front())) }
			};
		}
		stack.pop_front();
	}
	return !discard;
};

// ----------------------------------------------------------------------------

bool is_squeezing(const Contact* A, const Contact* B)
{
	float sepA = A->separation;
	float sepB = B->separation;
	float crush = sepA + sepB;
	bool any_has_contact = A->hasContact || B->hasContact;

	return crush >= 0.f
		&& any_has_contact
		&& A->ortho_n == -B->ortho_n;
}

bool is_crushing(const Contact* A, const Contact* B)
{
	Vec2f sum = A->collider_n + B->collider_n;
	return abs(sum.x) < 1e-5 && abs(sum.y) < 1e-5;
}

CollisionSolver::CompResult pickH(const Contact* east, const Contact* west, const Collidable* box)
{

	float eSep = east->separation;
	float wSep = west->separation;

	Rectf colBox = box->getBox();

	float crush = eSep + wSep;

	// diverging check
	if ((eSep >= colBox.width) != (wSep >= colBox.width)) {
		CollisionSolver::CompResult r;
		r.discardFirst = (eSep >= colBox.width);
		r.discardSecond = (wSep >= colBox.width);
		return r;
	}

	// crush check
	if (crush > 0.f) {
		CollisionSolver::CompResult r;
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

	// pick the one with contact
	if (east->hasContact != west->hasContact)
	{
		CollisionSolver::CompResult r;
		r.discardFirst = !east->hasContact;
		r.discardSecond = !west->hasContact;
		return r;
	}

	return {};
}

CollisionSolver::CompResult pickV(const Contact* north, const Contact* south, const Collidable* box)
{

	float nSep = north->separation;
	float sSep = south->separation;

	Rectf colBox = box->getBox();

	float crush = nSep + sSep;

	// diverging check
	if (crush > 0.f) {
		Vec2f nMid{ math::midpoint(north->collider.surface) };
		Vec2f sMid{ math::midpoint(south->collider.surface) };

		Vec2f nNormal = math::vector(north->collider.surface).lefthand().unit();
		bool nFacingAway = math::dot(sMid - nMid, nNormal) < 0;

		Vec2f sNormal = math::vector(south->collider.surface).lefthand().unit();
		bool sFacingAway = math::dot(nMid - sMid, sNormal) < 0;

		if ((nFacingAway && sFacingAway) || (crush > colBox.height)) {

			CollisionSolver::CompResult r;
			r.discardFirst = sSep < nSep;
			r.discardSecond = !r.discardFirst;
			return r;
		}
	}

	// crush check
	if (is_squeezing(north, south)
		&& is_crushing(north, south))
	{
		// crush
		CollisionSolver::CompResult r;
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
	// pick the one with contact
	if (north->hasContact != south->hasContact)
	{
		CollisionSolver::CompResult r;
		r.discardFirst = !north->hasContact;
		r.discardSecond = !south->hasContact;
		return r;
	}

	return {};
}

// ----------------------------------------------------------------------------

CollisionSolver::CompResult compare(const Contact* lhs, const Contact* rhs) {
	CollisionSolver::CompResult comp;

	if (lhs == rhs)
		return comp;

	const Contact& lhsContact = *lhs;
	const Contact& rhsContact = *rhs;

	comp.discardFirst = lhs->arbiter ? !lhs->arbiter->getCollision()->tileValid() : false;
	comp.discardSecond = rhs->arbiter ? !rhs->arbiter->getCollision()->tileValid() : false;

	// ghost check
	if (!comp.discardFirst && !comp.discardSecond)
	{
		GhostEdge g1 = isGhostEdge(rhsContact, lhsContact);
		GhostEdge g2 = isGhostEdge(lhsContact, rhsContact);

		bool g1_isGhost = (g1 != GhostEdge::None);
		bool g2_isGhost = (g2 != GhostEdge::None);

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

GhostEdge isGhostEdge(const Contact& basis, const Contact& candidate) noexcept
{
	if (!basis.isResolvable())
		return GhostEdge::None;

	bool isOneWay = (!basis.hasContact) && (basis.separation > 0.f) && (basis.impactTime == -1.0);

	Linef basisLine = basis.collider.surface;
	Linef candLine = candidate.collider.surface;

	Vec2f basisNormal = math::vector(basis.collider.surface).lefthand().unit();

	float dotp1 = math::dot(basisNormal, candLine.p1 - basisLine.p2);
	float dotp2 = math::dot(basisNormal, candLine.p2 - basisLine.p1);

	bool is_ghost = false;

	// candidate is full *behind* basis surface
	bool opt1;
	if (candLine.p1 == basisLine.p2
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
		return (opt1 ? GhostEdge::Full : GhostEdge::Partial);
	}
	else {
		return GhostEdge::None;
	}

}


CollisionSolver::CollisionSolver(Collidable* _collidable) 
	: collidable(_collidable)
{
}

void CollisionSolver::compareAll() 
{

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
				contacts.erase(contacts.begin() + i);
				i--;
				break;
			}
			else if (result.discardSecond) {
				contacts.erase(contacts.begin() + j);
			}
			else {
				j++;
			}
			cmp_count++;
		}
	}

}

void CollisionSolver::pushToAStack(Contact* contact)
{
	auto dir = direction::from_vector(contact->ortho_n);
	if (!dir.has_value()) {
		if (json_dump) {
			(*json_dump)["apply"] += {
				{ "discard_nodir", fmt::format("{}", fmt::ptr(contact)) }
			};
		}
	}
	else {
		switch (dir.value()) {
		case Cardinal::E:
			east.push_back(contact);
			break;
		case Cardinal::W:
			west.push_back(contact);
			break;
		case Cardinal::N:
			(contact->isTransposable() ? north_alt.push_back(contact) : north.push_back(contact));
			break;
		case Cardinal::S:
			(contact->isTransposable() ? south_alt.push_back(contact) : south.push_back(contact));
			break;
		}
	}
}

void CollisionSolver::pushToAStack(std::vector<Contact*> contacts)
{
	for (auto* contact : contacts)
	{
		pushToAStack(contact);
	}
}


std::optional<Contact> CollisionSolver::detectWedge(const Contact* north, const Contact* south)
{
	std::optional<Contact> contact;

	if (is_squeezing(north, south)
		&& !is_crushing(north, south))
	{
		Rectf colBox = collidable->getBox();

		Linef floorLine = north->collider.surface;
		Linef ceilLine = math::shift(south->collider.surface, Vec2f{ 0.f, colBox.height });

		Vec2f intersect = math::intersection(floorLine, ceilLine);

		if (std::isnan(intersect.x) || std::isnan(intersect.y)) 
		{
			LOG_WARN("bad intersection");
		}
		else if (intersect.x != collidable->getPosition().x) 
		{
			Vec2f pos = collidable->getPosition();
			float side = (north->collider_n.x + south->collider_n.x < 0.f ? -1.f : 1.f);

			contact = Contact{
				.separation = abs(intersect.x - pos.x),
				.hasContact = true,
				.position	= Vec2f{ pos.x, math::rect_mid(colBox).y },
				.ortho_n	= Vec2f{ side, 0.f },
				.collider_n = Vec2f{ side, 0.f },
				.velocity	= Vec2f{
					intersect - math::intersection(
						math::shift(floorLine, -north->velocity),
						math::shift(ceilLine, -south->velocity))
				}
			};
		}
	}
	return contact;
}

void CollisionSolver::detectWedges() {
	for (const auto* north_arb : north)
	{
		for (const auto* south_arb : south)
		{
			if (auto opt_contact = detectWedge(north_arb, south_arb))
			{
				if (auto dir = direction::from_vector(opt_contact->ortho_n))
				{
					created_contacts.push_back(*opt_contact);
					pushToAStack(&created_contacts.back());
				}
			}
		}
	}
}

std::vector<AppliedContact> CollisionSolver::solve(nlohmann::ordered_json* dump_ptr)
{
	frame.clear();

	json_dump = dump_ptr;

	if (contacts.empty())
		return {};

	if (json_dump)
	{
		for (auto& c : contacts) {
			(*json_dump)["precompare"] += to_json(c);
		}
		(*json_dump)["compare"];
	}

	// do contact-to-contact comparisons 
	// try to discard some redundant ones early
	compareAll();

	if (json_dump)
	{
		for (auto c : contacts)
		{
			(*json_dump)["postcompare"] += to_json(c);
		}
		(*json_dump)["apply"];
	}

	// organize contacts into north/east/south/west
	pushToAStack(contacts);
	contacts.clear();

	// detect any wedges
	// a wedge is any pair of north/south contacts that would force the collision box east/west instead
	detectWedges();

	// opportunistically determine if we can solve steeper slopes on the X axis instead of Y (looks nicer)
	if (canApplyAlt()) 
	{
		const auto transposeAndPush = [&](Contact* contact) -> void 
		{
			contact->transpose();
			pushToAStack(contact);
		};

		std::for_each(north_alt.begin(), north_alt.end(), transposeAndPush);
		std::for_each(south_alt.begin(), south_alt.end(), transposeAndPush);
	}
	else {
		// else bail out
		north.insert(north.end(), north_alt.begin(), north_alt.end());
		south.insert(south.end(), south_alt.begin(), south_alt.end());
	}
	north_alt.clear();
	south_alt.clear();

	std::sort(east.begin(), east.end(), ContactCompare);
	std::sort(west.begin(), west.end(), ContactCompare);

	if (json_dump) {
		(*json_dump)["apply"] += {
			{"x_stack", {
					{"east", to_json(east) },
					{"west", to_json(west) },
				}
			}
		};
	}

	// solve X axis
	if (solveAxis(east, west, pickH)) 
	{
		// update north/south contacts for our new X-pos
		updateStack(north);
		updateStack(south);

	}

	std::sort(north.begin(), north.end(), ContactCompare);
	std::sort(south.begin(), south.end(), ContactCompare);

	if (json_dump) {
		(*json_dump)["apply"] += {
			{"y_stack", {
					{"north", to_json(north) },
					{"south", to_json(south) },
				}
			}
		};
	}

	// solve Y axis
	solveAxis(north, south, pickV);

	return frame;
}

bool CollisionSolver::solveAxis(std::deque<Contact*>& stackA, std::deque<Contact*>& stackB, PickerFn picker)
{
	bool any_applied = false;

	while (!stackA.empty() && !stackB.empty()) {
		auto aC = stackA.front();
		auto bC = stackB.front();

		if (json_dump) {
			(*json_dump)["apply"] +=
			{
				{ "picking_from", {
					fmt::format("{}", fmt::ptr(aC)),
					fmt::format("{}", fmt::ptr(bC))
				}}
			};
		}

		auto r = picker(aC, bC, collidable);

		if (r.contact) {
			// picker generated a new contact
			if (apply(*r.contact, r.contactType))
			{
				any_applied = true;
				if (   r.contactType == ContactType::CRUSH_HORIZONTAL
					|| r.contactType == ContactType::CRUSH_VERTICAL) 
				{
					// nothing more we can do on this axis
					return true; 
				}
			}

			if (!r.discardFirst)  updateContact(aC);
			if (!r.discardSecond) updateContact(bC);

			r.discardFirst = !aC->hasContact;
			r.discardSecond = !bC->hasContact;
		}

		if (!r.discardFirst && !r.discardSecond)
		{
			// pick lowest separation to apply
			any_applied |= applyThenUpdateStacks(stackA, stackB, aC->separation < bC->separation);
		}
		else
		{
			// at least one will discard
			any_applied |= canApplyElseDiscard(r.discardFirst,  stackA) && applyThenUpdateStacks(stackA, stackB);
			any_applied |= canApplyElseDiscard(r.discardSecond, stackB) && applyThenUpdateStacks(stackB, stackA);
		}
	}

	// just one is non-empty, just apply the whole stack
	any_applied |= applyStack(stackA);
	any_applied |= applyStack(stackB);

	return any_applied;
}

// ----------------------------------------------------------------------------

bool CollisionSolver::applyStack(std::deque<Contact*>& stack) {
	bool any_applied = false;
	while (!stack.empty()) {
		if (applyFirst(stack))
		{
			any_applied = true;
			updateStack(stack);
		}
	}
	return any_applied;
}

bool CollisionSolver::applyFirst(std::deque<Contact*>& stack) {
	if (stack.empty())
		return false;

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
					}
				}
				else {
					if (abs(c2->position.x - mid.x) < abs(c1->position.x - mid.x)) {
						pick = it;
					}
				}
			}
		}
	}
	bool applied = apply(**pick);
	stack.erase(pick);

	return applied;
}

bool CollisionSolver::canApplyAlt() const
{
	auto isEast = [](auto c) { return c->collider_n.x > 0.f; };
	auto isWest = [](auto c) { return c->collider_n.x < 0.f; };

	auto isFlat = [](auto c) { return math::is_vertical(c->collider_n); };

	bool is_all_west = east.empty() 
		&& std::all_of(north_alt.cbegin(), north_alt.cend(), isWest)
		&& std::all_of(south_alt.cbegin(), south_alt.cend(), isWest);

	bool is_all_east = west.empty()
		&& std::all_of(north_alt.cbegin(), north_alt.cend(), isEast)
		&& std::all_of(south_alt.cbegin(), south_alt.cend(), isEast);

	bool floor_flat = std::all_of(north.cbegin(), north.cend(), isFlat);
	bool ceil_flat  = std::all_of(south.cbegin(), south.cend(), isFlat);

	return floor_flat && ceil_flat && (is_all_west || is_all_east);
}

// ----------------------------------------------------------------------------

bool CollisionSolver::apply(const Contact& contact, ContactType type)  
{
	bool has_contact = contact.hasContact;
	if (contact.hasContact) 
	{
		if (json_dump) {
			(*json_dump)["apply"] += to_json(&contact);
			(*json_dump)["apply"].back()["type"] = type;
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
		
	}
	else {
		if (json_dump) {
			(*json_dump)["apply"] += {
				{ "discard_apply", fmt::format("{}", fmt::ptr(&contact)) }
			};
		}
	}
	return has_contact;
}


}
