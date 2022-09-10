#pragma once

#include "fastfall/game_v2/phys/Collidable.hpp"
#include "fastfall/game_v2/phys/Arbiter.hpp"

#include <array>
#include <deque>

#include "nlohmann/json_fwd.hpp"

namespace ff {

enum class GhostEdge {
	None,
	Partial,
	Full
};

GhostEdge isGhostEdge(const ContinuousContact& basis, const ContinuousContact& candidate) noexcept;

class CollisionSolver {
public:
	// used in comparing contacts
	// may contain a generated contact
	struct CompResult {
		bool discardFirst = false;
		bool discardSecond = false;

        ContactType type = ContactType::NO_SOLUTION;
		std::optional<ContinuousContact> contact;
	};

private:
	// stacks that the contact get organized into
	std::deque<ContinuousContact*> north;
	std::deque<ContinuousContact*> south;
	std::deque<ContinuousContact*> east;
	std::deque<ContinuousContact*> west;

	// additional stacks for transposable north/south contacts
	std::vector<ContinuousContact*> north_alt;
	std::vector<ContinuousContact*> south_alt;

	// the collidable we're solving for
    ID<Collidable> collidable;

	// collision set of arbiters to solve
	std::vector<ContinuousContact> contacts;
	std::deque<ContinuousContact> created_contacts;

	// json ptr that the solver will optionally output state to
	// may be nullptr
	nlohmann::ordered_json* json_dump = nullptr;

	// vector of contacts that have been applied, in order of application
	std::vector<AppliedContact> frame;

	// pushes contact to appropriate north/south/east/west stack
	void pushToAStack(std::vector<ContinuousContact*> contact);
	void pushToAStack(ContinuousContact* contact);

	// arbiter may be nullptr
	// returns true if any contact is applied
	bool apply(const ContinuousContact& contact, ContactType type = ContactType::SINGLE);

	// returns true if any contact is applied
	bool applyStack(std::deque<ContinuousContact*>& stack);
	bool applyFirst(std::deque<ContinuousContact*>& stack);
	bool applyThenUpdateStacks(std::deque<ContinuousContact*>& stack, std::deque<ContinuousContact*>& otherStack, bool which = true);

	// pops front of stack if discard is true, returns !discard
	bool canApplyElseDiscard(bool discard, std::deque<ContinuousContact*>& stack);

	// returns true if any contact is applied
	using PickerFn = CompResult(*)(const ContinuousContact*, const ContinuousContact*, const Collidable*);
	bool solveAxis(std::deque<ContinuousContact*>& stackA, std::deque<ContinuousContact*>& stackB, PickerFn picker);

	// determine if internal state will allow steep contacts to be transposed
	bool canApplyAlt() const;

	// perform comparisons between all contacts in contacts
	void compareAll();

	// perform comparisons between all contacts in contacts
	void detectWedges();

	// compare two contacts to see if they form a wedge
	std::optional<ContinuousContact> detectWedge(const ContinuousContact* north, const ContinuousContact* south);

public:
	CollisionSolver(Collidable* _collidable);

	// add an arbiter associated with the collidable to the collision set
	inline void pushContact(ContinuousContact contact) { contacts.push_back(contact); };

	// attempts to resolve the combination of collisions
	// pushed contacts will be cleared after solving
	// returns vector of applied contact in order of application
	std::vector<AppliedContact> solve(nlohmann::ordered_json* dump_ptr = nullptr);

};

}
