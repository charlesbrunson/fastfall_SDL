#pragma once

namespace ff {

class InstanceID {
public:
	static constexpr unsigned NO_INSTANCE_ID = 0u;

	explicit InstanceID() :
		instanceID(NO_INSTANCE_ID)
	{

	}

	explicit InstanceID(unsigned int id) :
		instanceID(id)
	{

	}

	operator unsigned() const {
		return instanceID;
	}
	bool operator==(const InstanceID& rhs) {
		return instanceID == rhs.instanceID;
	}
private:
	unsigned int instanceID;
};

}
