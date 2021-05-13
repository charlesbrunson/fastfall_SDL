#pragma once


#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ff {

class Transform {
public:

	Transform(glm::fvec2 position = { 0.f, 0.f }, glm::fvec2 origin = { 0.f, 0.f }, glm::fvec2 scale = { 1.f, 1.f }, float rotation = 0.f);

	inline const glm::mat3&  getMatrix()   const { return m_transform_mat; };
	inline const glm::fvec2& getPosition() const { return m_position; };
	inline const glm::fvec2& getOrigin()   const { return m_origin; };
	inline const glm::fvec2& getScale()    const { return m_scale; };
	inline float			 getRotation() const { return m_rotation; };

	inline void setPosition(float posX, float posY) { setPosition({ posX, posY }); };
	inline void setOrigin(float originX, float originY) { setOrigin({originX, originY}); }
	inline void setScale(float scale) { setScale({ scale, scale }); };
	inline void setScale(float scaleX, float scaleY) { setScale({ scaleX, scaleY }); };

	void setPosition(glm::fvec2 position);
	void setOrigin(glm::fvec2 origin);
	void setScale(glm::fvec2 scale);
	void setRotation(float rotation);

	static Transform combine(const Transform& A, const Transform& B);
	static Transform inv_combine(const Transform& A, const Transform& B);

private:
	glm::mat3 m_transform_mat;

	glm::fvec2	m_position	= { 0.f, 0.f };
	glm::fvec2	m_origin	= { 0.f, 0.f };
	glm::fvec2	m_scale		= { 1.f, 1.f };
	float		m_rotation	= 0.f;

	void updateMatrix();
	
};

}