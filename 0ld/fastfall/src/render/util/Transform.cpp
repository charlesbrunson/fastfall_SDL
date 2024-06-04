#include "fastfall/render/util/Transform.hpp"

namespace ff {

	Transform::Transform(
		glm::fvec2 position, 
		glm::fvec2 origin, 
		glm::fvec2 scale, 
		float rotation
	)
		: m_position{ position },
		m_origin{ origin },
		m_scale{ scale },
		m_rotation{ rotation },
		m_transform_mat{ 1.f }
	{

	}

	void Transform::setPosition(glm::fvec2 position) { 
		m_position = position;
		updateMatrix();
	};

	void Transform::setOrigin(glm::fvec2 origin) {
		m_origin = origin;
		updateMatrix();
	};

	void Transform::setScale(glm::fvec2 scale) { 
		m_scale = scale;
		updateMatrix();
	};

	void Transform::setRotation(float radians) { 
		m_rotation = radians;
		updateMatrix();
	};

	Transform Transform::translate(glm::fvec2 shift) const noexcept {
		return Transform::combine(*this, Transform(shift));
	}
	Transform Transform::rotate(float radians) const noexcept {
		return Transform::combine(*this, Transform({}, {}, {}, radians));
	}
	Transform Transform::magnify(float scale) const noexcept {
		return Transform::combine(*this, Transform({}, {}, glm::fvec2{ scale, scale }, 0.f));
	}
	Transform Transform::magnify(glm::fvec2 scale) const noexcept {
		return Transform::combine(*this, Transform({}, {}, scale, 0.f));
	}

	void Transform::updateMatrix() {
		m_transform_mat = glm::translate(glm::mat3(1.0f), m_position * glm::fvec2(1.f, -1.f));
		m_transform_mat = glm::rotate(m_transform_mat, m_rotation);
		m_transform_mat = glm::scale(m_transform_mat, m_scale * glm::fvec2(1.f, -1.f));
		m_transform_mat = glm::translate(m_transform_mat, -m_origin);
	}

	Transform Transform::combine(const Transform& A, const Transform& B) {
		Transform tf = A;
		tf.m_position += B.m_position;
		tf.m_origin += B.m_origin;
		tf.m_scale *= B.m_scale;
		tf.m_rotation += B.m_rotation;
		tf.updateMatrix();

		return tf;
	}
	Transform Transform::inv_combine(const Transform& A, const Transform& B) {
		Transform tf = A;
		tf.m_position -= B.m_position;
		tf.m_origin -= B.m_origin;
		tf.m_scale /= B.m_scale;
		tf.m_rotation -= B.m_rotation;
		tf.updateMatrix();

		return tf;
	}


}