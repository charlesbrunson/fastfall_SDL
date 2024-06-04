#pragma once


#include "glm/glm.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace ff {

class Transform {
public:

	Transform(glm::fvec2 position = { 0.f, 0.f }, glm::fvec2 origin = { 0.f, 0.f }, glm::fvec2 scale = { 1.f, 1.f }, float rotation = 0.f);

	inline const glm::mat3&  getMatrix()   const noexcept { return m_transform_mat; };
	inline const glm::fvec2& getPosition() const noexcept { return m_position; };
	inline const glm::fvec2& getOrigin()   const noexcept { return m_origin; };
	inline const glm::fvec2& getScale()    const noexcept { return m_scale; };
	inline float			 getRotation() const noexcept { return m_rotation; };

	inline void setPosition(float posX, float posY) { setPosition({ posX, posY }); };
	inline void setOrigin(float originX, float originY) { setOrigin({originX, originY}); }
	inline void setScale(float scale) { setScale({ scale, scale }); };
	inline void setScale(float scaleX, float scaleY) { setScale({ scaleX, scaleY }); };

	void setPosition(glm::fvec2 position);
	void setOrigin(glm::fvec2 origin);
	void setScale(glm::fvec2 scale);
	void setRotation(float radians);

	[[nodiscard]] Transform translate(glm::fvec2 shift) const noexcept;
	[[nodiscard]] Transform rotate(float radians) const noexcept;
	[[nodiscard]] Transform magnify(float scale) const noexcept;
	[[nodiscard]] Transform magnify(glm::fvec2 scale) const noexcept;

	static Transform combine(const Transform& A, const Transform& B);
	static Transform inv_combine(const Transform& A, const Transform& B);

	inline bool operator== (const Transform& transform) {
		return m_transform_mat == transform.m_transform_mat;
	}

private:
	glm::mat3 m_transform_mat;

	glm::fvec2	m_position	= { 0.f, 0.f };
	glm::fvec2	m_origin	= { 0.f, 0.f };
	glm::fvec2	m_scale		= { 1.f, 1.f };
	float		m_rotation	= 0.f;

	void updateMatrix();
	
};

}