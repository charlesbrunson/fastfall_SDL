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

GhostEdge isGhostEdge(const Contact& basis, const Contact& candidate) noexcept;

class CollisionSolver {
public:
	// used in comparing contacts
	// may contain a generated contact
	struct CompResult {
		bool discardFirst = false;
		bool discardSecond = false;

		std::optional<Contact> contact;
	};

private:
	// stacks that the contact get organized into
	std::deque<Contact*> north;
	std::deque<Contact*> south;
	std::deque<Contact*> east;
	std::deque<Contact*> west;

	// additional stacks for transposable north/south contacts
	std::vector<Contact*> north_alt;
	std::vector<Contact*> south_alt;

	// the collidable we're solving for
	//Collidable* collidable = nullptr;
    ID<Collidable> collidable;

	// collision set of arbiters to solve
	std::vector<Contact> contacts;
	std::deque<Contact> created_contacts;

	// json ptr that the solver will optionally output state to
	// may be nullptr
	nlohmann::ordered_json* json_dump = nullptr;

	// vector of contacts that have been applied, in order of application
	std::vector<Contact> frame;

	// pushes contact to appropriate north/south/east/west stack
	void pushToAStack(std::vector<Contact*> contact);
	void pushToAStack(Contact* contact);

	// arbiter may be nullptr
	// returns true if any contact is applied
	bool apply(const Contact& contact, ContactType type = ContactType::SINGLE);

	// returns true if any contact is applied
	bool applyStack(std::deque<Contact*>& stack);
	bool applyFirst(std::deque<Contact*>& stack);
	bool applyThenUpdateStacks(std::deque<Contact*>& stack, std::deque<Contact*>& otherStack, bool which = true);

	// pops front of stack if discard is true, returns !discard
	bool canApplyElseDiscard(bool discard, std::deque<Contact*>& stack);

	// returns true if any contact is applied
	using PickerFn = CompResult(*)(const Contact*, const Contact*, const Collidable*);
	bool solveAxis(std::deque<Contact*>& stackA, std::deque<Contact*>& stackB, PickerFn picker);

	// determine if internal state will allow steep contacts to be transposed
	bool canApplyAlt() const;

	// perform comparisons between all contacts in contacts
	void compareAll();

	// perform comparisons between all contacts in contacts
	void detectWedges();

	// compare two contacts to see if they form a wedge
	std::optional<Contact> detectWedge(const Contact* north, const Contact* south);

public:
	CollisionSolver(Collidable* _collidable);

	// add an arbiter associated with the collidable to the collision set
	inline void pushContact(Contact contact) { contacts.push_back(contact); };

	// attempts to resolve the combination of collisions
	// pushed contacts will be cleared after solving
	// returns vector of applied contact in order of application
	std::vector<Contact> solve(nlohmann::ordered_json* dump_ptr = nullptr);

};

}
