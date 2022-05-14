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

	nlohmann::ordered_json to_json(const Arbiter* arb)
	{
		auto& contact = *arb->getContactPtr();
		return {
			{"arbiter",			fmt::format("{}", fmt::ptr(arb)) },
			{"hasContact",		contact.hasContact},
			{"separation",		contact.separation},
			{"ortho_n",			fmt::format("{}", contact.ortho_n)},
			{"collider_n",		fmt::format("{}", contact.collider_n)},
			{"hasImpactTime",	contact.hasImpactTime},
			{"impactTime",		contact.impactTime},
			{"velocity",		fmt::format("{}", contact.velocity)},
			{"region",			arb ? arb->region->get_ID().value : 0u},
			{"quad",			arb ? arb->collider->getID() : 0u}
		};
	}

	nlohmann::ordered_json to_json(const Contact& contact, const Arbiter* arb)
	{
		return {
			{"arbiter",			fmt::format("{}", fmt::ptr(arb)) },
			{"hasContact",		contact.hasContact},
			{"separation",		contact.separation},
			{"ortho_n",			fmt::format("{}", contact.ortho_n)},
			{"collider_n",		fmt::format("{}", contact.collider_n)},
			{"hasImpactTime",	contact.hasImpactTime},
			{"impactTime",		contact.impactTime},
			{"velocity",		fmt::format("{}", contact.velocity)},
			{"region",			arb ? arb->region->get_ID().value : 0u},
			{"quad",			arb ? arb->collider->getID() : 0u}
		};
	}

}


namespace ff {

bool CollisionSolver::canApplyAltArbiters(std::deque<Arbiter*>& north_alt, std::deque<Arbiter*>& south_alt) 
{
	auto isArbEast = [](const Arbiter* arb) {
		return arb->getContactPtr()->collider_n.x > 0.f;
	};

	auto isArbWest = [](const Arbiter* arb) {
		return arb->getContactPtr()->collider_n.x < 0.f;
	};

	bool allNorthAltWest = std::all_of(north_alt.cbegin(), north_alt.cend(), isArbWest);
	bool allSouthAltWest = std::all_of(south_alt.cbegin(), south_alt.cend(), isArbWest);

	bool allNorthAltEast = std::all_of(north_alt.cbegin(), north_alt.cend(), isArbEast);
	bool allSouthAltEast = std::all_of(south_alt.cbegin(), south_alt.cend(), isArbEast);

	bool allWest = east.empty() && allNorthAltWest && allSouthAltWest;
	bool allEast = west.empty() && allNorthAltEast && allSouthAltEast;

	return (!north_alt.empty() || !south_alt.empty()) && (allWest || allEast);
}

CollisionSolver::CollisionSolver(Collidable* _collidable) :
	collidable(_collidable),
	allCollisionCount({ 0u, 0u, 0u, 0u }),
	initialCollisionCount({ 0u, 0u, 0u, 0u }),
	appliedCollisionCount({ 0u, 0u, 0u, 0u }),
	applyCounter(0u)
{
}

void CollisionSolver::solve(nlohmann::ordered_json* dump_ptr) {

	json_dump = dump_ptr;

	if (arbiters.empty())
		return;

	if (json_dump)
	{
		size_t ndx = 0;
		for (auto& arb : arbiters) {
			(*json_dump)["precompare"][ndx] = to_json(arb);
			ndx++;
		}
	}

	// do arbiter-to-arbiter comparisons 

	if (json_dump && arbiters.size() == 1)
	{
		(*json_dump)["compare"];
	}

	size_t cmp_count = 0;
	for (size_t i = 0; i < arbiters.size() - 1; i++) {
		for (size_t j = i + 1; j < arbiters.size(); ) {

			ArbCompResult result = compArbiters(arbiters.at(i), arbiters.at(j));

			if (json_dump)
			{
				(*json_dump)["compare"][cmp_count] = {
					{"first",  fmt::format("{}", fmt::ptr(arbiters.at(i))) },
					{"second", fmt::format("{}", fmt::ptr(arbiters.at(j))) },
					{"result", {
							{"first",  (result.discardFirst ? "discard" : "keep")},
							{"second", (result.discardSecond ? "discard" : "keep")}
						}
					}
				};
			}

			if (result.discardFirst) {
				discard.push_back(std::make_pair(*(arbiters.begin() + i), result));
				arbiters.erase(arbiters.begin() + i);
				i--;
				break;
			}
			else if (result.discardSecond) {
				discard.push_back(std::make_pair(*(arbiters.begin() + j), result));
				arbiters.erase(arbiters.begin() + j);
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
		for (auto& arb : arbiters) {
			auto& contact = *arb->getContactPtr();

			(*json_dump)["postcompare"][ndx] = to_json(arb);
			ndx++;
		}
	}

	if (json_dump)
	{
		(*json_dump)["apply"];
	}

	// initialize arbiter lists
	std::deque<Arbiter*> north_alt;
	std::deque<Arbiter*> south_alt;

	for (auto* arb : arbiters) {
		const Contact* contact = arb->getContactPtr();
		Vec2f oN = contact->ortho_n;
		Vec2f cN = contact->collider_n;


		auto dir = direction::from_vector(oN);
		if (!dir.has_value()) {
			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(arb));
				applyCounter++;
			}
			discard.push_back(std::make_pair(arb, ArbCompResult{}));
			continue;
		}

		allCollisionCount[dir.value()]++;
		if (contact->hasContact)
			initialCollisionCount[dir.value()]++;

		switch (dir.value()) {
		case Cardinal::E:
			east.push_back(arb);
			break;
		case Cardinal::W:
			west.push_back(arb);
			break;
		case Cardinal::N:
			(contact->isTransposable() ? north_alt : north).push_back(arb);
			break;
		case Cardinal::S:
			(contact->isTransposable() ? south_alt : south).push_back(arb);
			break;
		}
	}

	if (canApplyAltArbiters(north_alt, south_alt)) {
		applyArbVertAsHori(north_alt, north);
		applyArbVertAsHori(south_alt, south);
	}
	else {
		// bail out
		north.insert(north.end(), north_alt.begin(), north_alt.end());
		south.insert(south.end(), south_alt.begin(), south_alt.end());
		north_alt.clear();
		south_alt.clear();
	}



	// solve X axis
	solveX();

	// solve Y axis
	solveY();

}


void CollisionSolver::solveX() {

	for (auto arb : east) {
		arb->update(0.0);
	}
	for (auto arb : west) {
		arb->update(0.0);
	}

	std::sort(east.begin(), east.end(), ArbiterCompare);
	std::sort(west.begin(), west.end(), ArbiterCompare);

	while (!east.empty() && !west.empty()) {
		auto eastArb = east.front();
		auto westArb = west.front();

		auto r = pickHArbiter(eastArb, westArb);

		if (r.contact) {
			apply(*r.contact, nullptr, r.contactType);

			if (r.contactType == ContactType::CRUSH_HORIZONTAL)
				return;

			if (!r.discardFirst)  eastArb->update(0.0);
			if (!r.discardSecond) westArb->update(0.0);
		}

		if (r.discardFirst && !r.discardSecond) {

			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(east.front()));
				applyCounter++;
			}

			east.pop_front();
			applyArbiterFirst(west);
			updateArbiterStack(east);
		}
		else if (r.discardSecond && !r.discardFirst) {

			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(west.front()));
				applyCounter++;
			}

			west.pop_front();
			applyArbiterFirst(east);
			updateArbiterStack(west);
		}
		else if (r.discardFirst && r.discardSecond) {

			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(east.front()));
				applyCounter++;
				(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(west.front()));
				applyCounter++;
			}

			east.pop_front();
			west.pop_front();
		}
		else {
			if (eastArb->getContactPtr()->separation < westArb->getContactPtr()->separation) {
				applyArbiterFirst(east);
				updateArbiterStack(west);
			}
			else {
				applyArbiterFirst(west);
				updateArbiterStack(east);
			}
		}
	}

	if (!east.empty()) {
		applyArbiterStack(east);
	}
	else if (!west.empty()) {
		applyArbiterStack(west);
	}
}

void CollisionSolver::solveY() {

	for (auto arb : north) {
		arb->update(0.0);
	}
	for (auto arb : south) {
		arb->update(0.0);
	}

	std::sort(north.begin(), north.begin(), ArbiterCompare);
	std::sort(south.begin(), south.begin(), ArbiterCompare);
	
	while (!north.empty() && !south.empty()) {
		auto northArb = north.front();
		auto southArb = south.front();

		auto r = pickVArbiter(northArb, southArb);

		if (r.contact) {
			apply(*r.contact, nullptr, r.contactType);

			if (r.contactType == ContactType::CRUSH_VERTICAL)
				return;

			if (!r.discardFirst)  northArb->update(0.0);
			if (!r.discardSecond) southArb->update(0.0);

			if (!northArb->getContactPtr()->hasContact
				&& !southArb->getContactPtr()->hasContact) 
			{
				if (json_dump) {
					(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(north.front()));
					applyCounter++;
					(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(south.front()));
					applyCounter++;
				}
				north.pop_front();
				south.pop_front();
				continue;
			}

			if (!northArb->getContactPtr()->hasContact) {
				if (json_dump) {
					(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(north.front()));
					applyCounter++;
				}
				north.pop_front();
				continue;
			}
			if (!southArb->getContactPtr()->hasContact) {
				if (json_dump) {
					(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(south.front()));
					applyCounter++;
				}
				south.pop_front();
				continue;
			}

		}

		if (r.discardFirst && !r.discardSecond) {

			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(north.front()));
				applyCounter++;
			}

			north.pop_front();
			applyArbiterFirst(south);
			updateArbiterStack(north);
		}
		else if (r.discardSecond && !r.discardFirst) {
			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(south.front()));
				applyCounter++;
			}

			south.pop_front();
			applyArbiterFirst(north);
			updateArbiterStack(south);
		}
		else if (r.discardFirst && r.discardSecond) {

			if (json_dump) {
				(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(north.front()));
				applyCounter++;
				(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(south.front()));
				applyCounter++;
			}

			north.pop_front();
			south.pop_front();
		}
		else {
			if (northArb->getContactPtr()->separation < southArb->getContactPtr()->separation) {
				applyArbiterFirst(north);
				updateArbiterStack(south);
			}
			else {
				applyArbiterFirst(south);
				updateArbiterStack(north);
			}
		}
	}

	if (!north.empty()) {
		applyArbiterStack(north);
	}
	else if (!south.empty()) {
		applyArbiterStack(south);
	}
}


// ----------------------------------------------------------------------------



void CollisionSolver::updateArbiterStack(std::deque<Arbiter*>& stack) {
	for (auto arb = stack.begin(); arb != stack.end(); arb++) {
		(*arb)->update(0.0);
	}
	std::sort(stack.begin(), stack.end(), ArbiterCompare);
}

void CollisionSolver::applyArbiterStack(std::deque<Arbiter*>& stack) {
	while (!stack.empty()) {
		applyArbiterFirst(stack);
	}
}

void CollisionSolver::applyArbiterFirst(std::deque<Arbiter*>& stack) {
	if (stack.empty())
		return;

	// if multiple arbiters with the same sep, prefer the one closest to the collidable's center
	std::deque<Arbiter*>::iterator pick = stack.begin();
	const Contact* c = (*pick)->getContactPtr();
	if (stack.size() > 1) 
	{
		float sep = c->separation;

		for (auto it = stack.begin() + 1; it != stack.end(); it++) 
		{
			if ((*it)->getContactPtr()->separation != sep) {
				break;
			}
			else {
				const Contact* c1 = (*pick)->getContactPtr();
				const Contact* c2 = (*it)->getContactPtr();
				Vec2f mid = math::rect_mid(collidable->getBox());

				if (math::is_horizontal(c->ortho_n)) {
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
	apply(*(*pick)->getContactPtr(), *pick);
	stack.erase(pick);

	updateArbiterStack(stack);
}

void CollisionSolver::applyArbVertAsHori(std::deque<Arbiter*>& altList, std::deque<Arbiter*>& backupList) {

	while (!altList.empty()) {

		auto* arb = altList.front();
		const Contact* c = arb->getContactPtr();

		bool updateRemaining = false;

		if (c->hasContact) {
			//assert(abs(c->collider_n.x) > 0.f && abs(c->collider_n.x) < 1.f);

			Vec2f alt_ortho_normal = (c->collider_n.x < 0.f ? Vec2f(-1.f, 0.f) : Vec2f(1.f, 0.f));
			float alt_separation = abs((c->collider_n.y * c->separation) / c->collider_n.x);

			ArbCompResult r;
			r.contactType = ContactType::SINGLE;
			r.contact = *c;
			r.contact->ortho_n = alt_ortho_normal;
			r.contact->separation = alt_separation;
			apply(*r.contact, nullptr, r.contactType);
			updateRemaining = true;
		}

		altList.pop_front();

		if (updateRemaining) {
			for (auto* arb : altList) {
				arb->update(0.0);
			}
		}
	}
}

// ----------------------------------------------------------------------------

void CollisionSolver::apply(const Contact& contact, Arbiter* arbiter, ContactType type)  {


	if (contact.hasContact) {
		appliedCollisionCount[direction::from_vector(contact.ortho_n).value()]++;

		if (json_dump) {
			(*json_dump)["apply"][applyCounter] = to_json(contact, arbiter);
			(*json_dump)["apply"][applyCounter]["type"] = type;
		}

		collidable->applyContact(contact, type);

		AppliedContact applied;
		applied.arbiter = arbiter;
		applied.contact = contact;
		applied.type = type;

		if (arbiter != nullptr) {
			arbiter->setApplied();
			arbiter->update(0.0);
			applied.region = arbiter->region;
		}

		frame.push_back(applied);
		applyCounter++;
	}
	else {
		if (json_dump) {
			(*json_dump)["apply"][applyCounter]["discard"] = fmt::format("{}", fmt::ptr(arbiter));
			applyCounter++;
		}
	}
}

// ----------------------------------------------------------------------------


CollisionSolver::ArbCompResult CollisionSolver::compArbiters(const Arbiter* lhs, const Arbiter* rhs) {
	ArbCompResult comp;

	if (lhs == rhs)
		return comp;

	const Contact& lhsContact = *lhs->getContactPtr();
	const Contact& rhsContact = *rhs->getContactPtr();

	comp.discardFirst  = !lhs->getCollision()->tileValid();
	comp.discardSecond = !rhs->getCollision()->tileValid();

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
					g1_isGhost = !ArbiterCompare(lhs, rhs);
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

CollisionSolver::ArbCompResult CollisionSolver::pickHArbiter(const Arbiter* east, const Arbiter* west) {

	const Contact* eContact = east->getContactPtr();
	const Contact* wContact = west->getContactPtr();

	float eSep = eContact->separation;
	float wSep = wContact->separation;

	Rectf colBox = collidable->getBox();

	float crush = eContact->separation + wContact->separation;

	// diverging check
	if (eSep > colBox.width && wSep < colBox.width) {
		ArbCompResult r;
		r.discardFirst = true;
		r.discardSecond = false;
		return r;
	}
	else if (eSep < colBox.width && wSep > colBox.width) {
		ArbCompResult r;
		r.discardFirst = false;
		r.discardSecond = true;
		return r;
	}

	// crush
	if (crush > 0.f) {
		ArbCompResult r;
		r.contactType = ContactType::CRUSH_HORIZONTAL;

		r.discardFirst = true;
		r.discardSecond = true;

		if (eSep > wSep) {
			r.contact = *eContact;
			r.contact->separation -= wSep;
			r.contact->separation /= 2.f;
		}
		else {
			r.contact = *wContact;
			r.contact->separation -= eSep;
			r.contact->separation /= 2.f;
		}
		return r;
	}

	if (!eContact->hasContact)
	{
		ArbCompResult r;
		r.discardFirst = true;
		r.discardSecond = false;
		return r;
	}
	else if (!wContact->hasContact)
	{
		ArbCompResult r;
		r.discardFirst = false;
		r.discardSecond = true;
		return r;
	}
	return ArbCompResult{};
}

CollisionSolver::ArbCompResult CollisionSolver::pickVArbiter(const Arbiter* north, const Arbiter* south) {

	const Contact* nContact = north->getContactPtr();
	const Contact* sContact = south->getContactPtr();

	float nSep = nContact->separation;
	float sSep = sContact->separation;

	Rectf colBox = collidable->getBox();

	float crush = nSep + sSep;

	bool any_has_contact = nContact->hasContact || sContact->hasContact;

	// diverging check
	if (crush > 0.f) {
		Vec2f nMid{ math::midpoint(nContact->collider.surface) };
		Vec2f sMid{ math::midpoint(sContact->collider.surface) };

		Vec2f nNormal = math::vector(nContact->collider.surface).lefthand().unit();
		bool nFacingAway = math::dot(sMid - nMid, nNormal) < 0;

		Vec2f sNormal = math::vector(sContact->collider.surface).lefthand().unit();
		bool sFacingAway = math::dot(nMid - sMid, sNormal) < 0;

		if ((nFacingAway && sFacingAway) || (crush > colBox.height)) {

			if (nSep < sSep) {
				ArbCompResult r;
				r.discardFirst = false;
				r.discardSecond = true;
				return r;
			}
			else {
				ArbCompResult r;
				r.discardFirst = true;
				r.discardSecond = false;
				return r;
			}
		}
	}

	// arbs are converging
	if (crush >= 0.f && any_has_contact
		&& nContact->ortho_n == -sContact->ortho_n) 
	{

		Vec2f sum = nContact->collider_n + sContact->collider_n;

		if (   abs(sum.x) < 1e-5
			&& abs(sum.y) < 1e-5)
		{

			// crush
			ArbCompResult r;
			r.contactType = ContactType::CRUSH_VERTICAL;

			r.discardFirst = true;
			r.discardSecond = true;

			if (nSep > sSep) {
				r.contact = *nContact;
				r.contact->separation -= sSep;
				r.contact->separation /= 2.f;
			}
			else {
				r.contact = *sContact;
				r.contact->separation -= nSep;
				r.contact->separation /= 2.f;
			}
			return r;
		}
		// wedge
		else {

			Angle floorAng = math::angle(nContact->collider.surface);
			Angle ceilAng  = math::angle(sContact->collider.surface);

			Linef floorLine = nContact->collider.surface;
			Linef ceilLine = math::shift(sContact->collider.surface, Vec2f{ 0.f, colBox.height });

			Vec2f floorVel = nContact->velocity;
			Vec2f ceilVel  = sContact->velocity;

			Vec2f intersect = math::intersection(floorLine, ceilLine);

			if (std::isnan(intersect.x) || std::isnan(intersect.y)) {
				LOG_WARN("bad intersection");
			}
			else if (intersect.x != collidable->getPosition().x) {

				ArbCompResult r;
				//r.createdContact = true;
				r.contactType = ContactType::WEDGE;

				r.discardFirst = false;
				r.discardSecond = false;

				Vec2f pos = collidable->getPosition();

				Vec2f diff = Vec2f(intersect.x, 0.f) - Vec2f(pos.x, 0.f);
				float side = (nContact->collider_n.x + sContact->collider_n.x < 0.f ? -1.f : 1.f);

				r.contact = Contact{};
				r.contact->hasContact = true;
				r.contact->separation = abs(diff.x);
				r.contact->collider_n = Vec2f{ side, 0.f };
				r.contact->ortho_n = r.contact->collider_n;
				r.contact->position = Vec2f{ pos.x, math::rect_mid(colBox).y };

				r.contact->velocity = intersect - math::intersection(
					math::shift(floorLine, -floorVel), 
					math::shift(ceilLine, -ceilVel)
				);

				return r;
			}
			//else do nothing
		}
	}
	if (any_has_contact && !nContact->hasContact && sContact->hasContact)
	{
		ArbCompResult r;
		r.discardFirst = true;
		r.discardSecond = false;
		return r;
	}
	else if (any_has_contact && !sContact->hasContact && nContact->hasContact)
	{
		ArbCompResult r;
		r.discardFirst = false;
		r.discardSecond = true;
		return r;
	}

	ArbCompResult r;
	r.discardFirst = false;
	r.discardSecond = false;
	return r;
}

}
