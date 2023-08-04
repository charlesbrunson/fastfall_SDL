#include "fastfall/game/phys/CollisionSolver.hpp"

#include "fastfall/game/phys/CollidableArbiter.hpp"

#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"

#include "nlohmann/json.hpp"

namespace ff 
{

// json helper utils

NLOHMANN_JSON_SERIALIZE_ENUM(ContactType, {
	{ContactType::NO_SOLUTION,		"no solution"},
	{ContactType::SINGLE,			"single"},
	{ContactType::WEDGE,			"wedge"},
	{ContactType::CRUSH_HORIZONTAL,	"horizontal crush"},
	{ContactType::CRUSH_VERTICAL,	"vertical crush"},
	})

nlohmann::ordered_json to_json(const ContinuousContact* contact)
{
	//const auto* arb = contact->arbiter;
    auto id = contact->id;
	return {
		{"contact",			fmt::format("{}", fmt::ptr(contact)) },
		{"hasContact",		contact->hasContact},
		{"separation",		contact->separation},
        {"surface",	        fmt::format("{} -> {}", contact->collider.surface.p1, contact->collider.surface.p2)},
		{"ortho_n",			fmt::format("{}", contact->ortho_n)},
		{"collider_n",		fmt::format("{}", contact->collider_n)},
		{"hasImpactTime",	contact->hasImpactTime},
		{"impactTime",		contact->impactTime},
		{"velocity",		fmt::format("{}", contact->velocity)},
		{"is_transposed",	contact->is_transposed},
        {"collidable",		id ? id->collidable.raw() : 0u },
		{"region",			id ? id->collider.raw() : 0u },
		{"quad",			id ? id->quad.value : 0u },
        {"stick_offset",    contact->stickOffset},
        {"stick_line",      fmt::format("{} -> {}", contact->stickLine.p1, contact->stickLine.p2)},
    };
}

template<typename Container>
requires std::same_as<typename Container::value_type, ContinuousContact*>
nlohmann::ordered_json to_json(Container container)
{
	nlohmann::ordered_json json;
	for (const ContinuousContact* c : container)
	{
		json += fmt::format("{}", fmt::ptr(c));
	}
	return json;
}

// solver utils

void CollisionSolver::updateContact(ContinuousContact* contact)
{
    auto arb = contact->id ? arbiters.find(*contact->id) : arbiters.end();
	if (arb != arbiters.end()) {
        arb->second->update({
                .collider = colliders->get(contact->id->collider),
                .collidable = collidable
        }, 0.0);
	}
}

void CollisionSolver::updateStack(std::deque<ContinuousContact*>& stack) {
	std::for_each(stack.begin(), stack.end(), std::bind(&CollisionSolver::updateContact, this, std::placeholders::_1));
}


// ----------------------------------------------------------------------------

// utils for detecting crush/wedges

bool is_squeezing(const ContinuousContact* A, const ContinuousContact* B)
{
	float sepA = A->separation;
	float sepB = B->separation;
	float crush = sepA + sepB;
	bool any_has_contact = A->hasContact || B->hasContact;

	return crush >= 0.f
		&& any_has_contact
		&& A->ortho_n == -B->ortho_n;
}

bool is_crushing(const ContinuousContact* A, const ContinuousContact* B)
{
	Vec2f sum = A->collider_n + B->collider_n;
	return abs(sum.x) < 1e-5 && abs(sum.y) < 1e-5;
}

bool is_diverging_V(Linef A, Linef B)
{
	Vec2f nMid{ math::midpoint(A) };
	Vec2f sMid{ math::midpoint(B) };

	Vec2f nNormal = math::vector(A).lefthand().unit();
	bool nFacingAway = math::dot(sMid - nMid, nNormal) < 0;

	Vec2f sNormal = math::vector(B).lefthand().unit();
	bool sFacingAway = math::dot(nMid - sMid, sNormal) < 0;

	return nFacingAway && sFacingAway;
}

// solve functions for x- and y-axis

CollisionSolver::CompResult pickH(const ContinuousContact* east, const ContinuousContact* west, const Collidable* box)
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
		r.type = ContactType::CRUSH_HORIZONTAL;

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

CollisionSolver::CompResult pickV(const ContinuousContact* north, const ContinuousContact* south, const Collidable* box)
{

	float nSep = north->separation;
	float sSep = south->separation;

	Rectf colBox = box->getBox();

	float crush = nSep + sSep;

	// diverging check
	if (crush > colBox.height
		|| is_diverging_V(north->collider.surface, south->collider.surface))
	{
		CollisionSolver::CompResult r;
		r.discardFirst = sSep < nSep;
		r.discardSecond = !r.discardFirst;
		return r;
	}

	// crush check
	if (is_squeezing(north, south)
		&& is_crushing(north, south))
	{
		// crush
		CollisionSolver::CompResult r;
		r.type = ContactType::CRUSH_VERTICAL;
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

CollisionSolver::CollisionSolver(poly_id_map<ColliderRegion>* _colliders, Collidable* _collidable)
	: collidable(_collidable)
    , colliders(_colliders)
{
}

bool CollisionSolver::applyThenUpdateStacks(std::deque<ContinuousContact*>& stack, std::deque<ContinuousContact*>& otherStack, bool which)
{
	bool applied = applyFirst(which ? stack : otherStack);
	if (applied) {
		updateStack(stack);
		updateStack(otherStack);
	}
	return applied;
};

bool CollisionSolver::canApplyElseDiscard(bool discard, std::deque<ContinuousContact*>& stack)
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

void CollisionSolver::pushToAStack(ContinuousContact* contact)
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
			(contact->transposable() ? north_alt.push_back(contact) : north.push_back(contact));
			break;
		case Cardinal::S:
			(contact->transposable() ? south_alt.push_back(contact) : south.push_back(contact));
			break;
		}
	}
}

void CollisionSolver::pushToAStack(std::vector<ContinuousContact*>& t_contacts)
{
	for (auto* contact : t_contacts)
	{
		pushToAStack(contact);
	}
}

CollisionSolver::CompResult compare(const ContinuousContact* lhs, const ContinuousContact* rhs) {
	CollisionSolver::CompResult comp;

	if (lhs == rhs)
		return comp;

	const ContinuousContact& lhsContact = *lhs;
	const ContinuousContact& rhsContact = *rhs;

	comp.discardFirst = !lhs->quad_valid;
	comp.discardSecond = !rhs->quad_valid;

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
					// double ghost, pick the one with the least separation
                    //g1_isGhost = !compare_contact(*lhs, *rhs);
                    auto order = compare_contact(*lhs, *rhs);
                    if (order == std::weak_ordering::equivalent) {
                        g1_isGhost = false;
                        g2_isGhost = false;
                    }
                    else {
                        g1_isGhost = order == std::weak_ordering::greater;
                        g2_isGhost = !g1_isGhost;
                    }
				}
				else {
					g1_isGhost = g2 < g1;
                    g2_isGhost = !g1_isGhost;
				}
			}

			comp.discardFirst  = g1_isGhost;
			comp.discardSecond = g2_isGhost;
		}
	}
	return comp;
}

GhostEdge isGhostEdge(const ContinuousContact& basis, const ContinuousContact& candidate) noexcept
{
	if (!basis.is_resolvable())
		return GhostEdge::None;

	bool isOneWay = (!basis.hasContact) && (basis.separation > 0.f) && (basis.impactTime == -1.0);

	Linef basisLine = basis.collider.surface;
	Linef candLine = candidate.collider.surface;

	Vec2f basisNormal = math::vector(basis.collider.surface).lefthand().unit();

	float dotp1 = math::dot(basisNormal, candLine.p1 - basisLine.p2);
	float dotp2 = math::dot(basisNormal, candLine.p2 - basisLine.p1);
    //LOG_INFO("dot1:{} dot2:{}", dotp1, dotp2);
	 
	//bool is_ghost = false;

    bool shares_p1 = basisLine.p1 == candLine.p2;
    bool shares_p2 = basisLine.p2 == candLine.p1;

	// candidate is full *behind* basis surface
	bool opt1;

	if (shares_p1) {
		// surfaces share a point
		opt1 = (dotp1 < 0.f && dotp2 <= 0.f);
	}
    else if (shares_p2) {
        // surfaces share a point
        opt1 = (dotp1 <= 0.f && dotp2 < 0.f);
    }


    /*
    if (shares_p1 || shares_p2) {
        opt1 = (dotp1 < 0.f && dotp2 < 0.f);
    }
    */
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

void CollisionSolver::compareAll() 
{

	size_t cmp_count = 0;
	for (size_t i = 0; i < contacts.size() - 1; i++) {
		for (size_t j = i + 1; j < contacts.size(); ) {

			CompResult result = compare(contacts.at(i), contacts.at(j));

			if (json_dump)
			{
				(*json_dump)["compare"][cmp_count] = {
					{"first",  fmt::format("{}", fmt::ptr(&contacts.at(i))) },
					{"second", fmt::format("{}", fmt::ptr(&contacts.at(j))) },
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

std::optional<ContinuousContact> CollisionSolver::detectWedge(const ContinuousContact* north, const ContinuousContact* south)
{
	std::optional<ContinuousContact> contact;

	if (is_squeezing(north, south)
		&& !is_crushing(north, south)
		&& !is_diverging_V(north->collider.surface, south->collider.surface))
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

			contact = ContinuousContact{};
            contact->id = std::nullopt;
            contact->separation = abs(intersect.x - pos.x),
            contact->hasContact = true,
            contact->position	= Vec2f{ pos.x, math::rect_mid(colBox).y },
            contact->ortho_n	= Vec2f{ side, 0.f },
            contact->collider_n = Vec2f{ side, 0.f },
            contact->velocity	= Vec2f{
                intersect - math::intersection(
                    math::shift(floorLine, -north->velocity),
                    math::shift(ceilLine, -south->velocity))
            };

			if (json_dump)
			{
				(*json_dump)["wedges"] += {
					{"north", fmt::format("{}", fmt::ptr(north)) },
					{"south", fmt::format("{}", fmt::ptr(south)) },
					{"created_contact", to_json(&*contact)}
				};
			}
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

std::vector<AppliedContact>&& CollisionSolver::solve(nlohmann::ordered_json* dump_ptr)
{
	frame.clear();

	json_dump = dump_ptr;

    //std::transform(contacts);

	if (contacts.empty())
		return std::move(frame);

	if (json_dump)
	{
		for (auto* contact : contacts) {
			(*json_dump)["precompare"] += to_json(contact);
		}
		(*json_dump)["compare"];
	}

	// do contact-to-contact comparisons 
	// try to discard some redundant ones early
	compareAll();

	if (json_dump)
	{
		for (auto* contact : contacts)
		{
			(*json_dump)["postcompare"] += to_json(contact);
		}
	}

	// organize contacts into north/east/south/west
    for (auto* contact : contacts)
    {
        pushToAStack(contact);
    }
	//contacts.clear();

	// detect any wedges
	// a wedge is any pair of north/south contacts that would force the collision box east/west instead
	detectWedges();

	if (json_dump)
		(*json_dump)["apply"];

	// opportunistically determine if we can solve steeper slopes on the X axis instead of Y (looks nicer)
	if (canApplyAlt()) 
	{
		const auto transposeAndPush = [&](ContinuousContact* contact) -> void
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

    auto cmp_contact_ptrs = [](auto* lhs, auto* rhs) { return compare_contact(*lhs, *rhs) == std::weak_ordering::less; };

	std::sort(east.begin(), east.end(), cmp_contact_ptrs);
	std::sort(west.begin(), west.end(), cmp_contact_ptrs);

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

	std::sort(north.begin(), north.end(), cmp_contact_ptrs);
	std::sort(south.begin(), south.end(), cmp_contact_ptrs);

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

	return std::move(frame);
}

bool CollisionSolver::solveAxis(std::deque<ContinuousContact*>& stackA, std::deque<ContinuousContact*>& stackB, PickerFn picker)
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
			if (apply(*r.contact, r.type))
			{
				any_applied = true;
				if (   r.type == ContactType::CRUSH_HORIZONTAL
					|| r.type == ContactType::CRUSH_VERTICAL)
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

bool CollisionSolver::applyStack(std::deque<ContinuousContact*>& stack) {
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

bool CollisionSolver::applyFirst(std::deque<ContinuousContact*>& stack) {
	if (stack.empty())
		return false;

	// if there are multiple arbiters with the same sep, prefer the one closest to the collidable's center
	std::deque<ContinuousContact*>::iterator pick = stack.begin();
	if (stack.size() > 1) 
	{
		float sep = (*pick)->separation;

		for (auto it = stack.begin() + 1; it != stack.end(); it++) 
		{
			if ((*it)->separation != sep) {
				break;
			}
			else {
				const ContinuousContact* c1 = *pick;
				const ContinuousContact* c2 = *it;
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

bool CollisionSolver::apply(const ContinuousContact& contact, ContactType type)
{
	bool has_contact = contact.hasContact;
	if (contact.hasContact) 
	{
		if (json_dump) {
			(*json_dump)["apply"] += to_json(&contact);
			(*json_dump)["apply"].back()["type"] = type;
		}

        Vec2f prevel = collidable->get_global_vel();
		collidable->applyContact(contact, type);

		AppliedContact applied{contact};
        applied.collidable_precontact_velocity = prevel;
        //LOG_INFO("{}", applied.collidable_precontact_velocity);
		applied.type = type;

        auto arb = contact.id ? arbiters.find(*contact.id) : arbiters.end();
		if (arb != arbiters.end()) {
			arb->second->setApplied();
			arb->second->update({
                .collider = colliders->get(contact.id->collider),
                .collidable = collidable
            }, 0.0);
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
