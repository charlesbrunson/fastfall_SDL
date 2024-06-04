#pragma once

#include "fastfall/render/util/Transform.hpp"
#include "fastfall/render/util/Texture.hpp"
#include "fastfall/render/util/Shader.hpp"
#include "fastfall/render/util/Color.hpp"

#include "fastfall/render/external/opengl.hpp"

namespace ff {


class BlendMode {
public:
	enum class Factor {
		ZERO					= GL_ZERO,
		ONE						= GL_ONE,
		SRC_COLOR				= GL_SRC_COLOR,
		ONE_MINUS_SRC_COLOR		= GL_ONE_MINUS_SRC_COLOR,
		DST_COLOR				= GL_DST_COLOR,
		ONE_MINUS_DST_COLOR		= GL_ONE_MINUS_DST_COLOR,
		SRC_ALPHA				= GL_SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA		= GL_ONE_MINUS_SRC_ALPHA,
		DST_ALPHA				= GL_DST_ALPHA,
		ONE_MINUS_DST_ALPHA		= GL_ONE_MINUS_DST_ALPHA,
		CONSTANT_COLOR			= GL_CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR= GL_ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA			= GL_CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA= GL_ONE_MINUS_CONSTANT_ALPHA,
		SRC_ALPHA_SATURATE		= GL_SRC_ALPHA_SATURATE,
		SRC1_COLOR				= GL_SRC1_COLOR,
		ONE_MINUS_SRC1_COLOR	= GL_ONE_MINUS_SRC1_COLOR,
		SRC1_ALPHA				= GL_SRC1_ALPHA,
		ONE_MINUS_SRC1_ALPHA	= GL_ONE_MINUS_SRC1_ALPHA
	};
	enum class Equation {
		ADD				= GL_FUNC_ADD,
		SUBTRACT		= GL_FUNC_SUBTRACT,
		REVERSE_SUBTRACT= GL_FUNC_REVERSE_SUBTRACT,
		MIN				= GL_MIN,
		MAX				= GL_MAX
	};

	BlendMode();
	BlendMode(Factor srcFactor, Factor dstFactor, Equation blendEquation);
	BlendMode(Factor srcFactorColor, Factor dstFactorColor, Equation blendEquationColor,
			  Factor srcFactorAlpha, Factor dstFactorAlpha, Equation blendEquationAlpha);


	void setBlendEquation(Equation equationRGBA);
	void setBlendEquation(Equation equationColor, Equation equationAlpha);

	void setSrcFactor(Factor factorRGBA);
	void setSrcFactor(Factor factorColor, Factor factorAlpha);

	void setDstFactor(Factor factorRGBA);
	void setDstFactor(Factor factorColor, Factor factorAlpha);
	void setColor(ff::Color color);

	inline Equation getEquationRGB() const { return equationRGB; };
	inline Equation getEquationA() const { return equationRGB; };

	inline Factor getSrcFactorRGB() const { return srcFactorRGB; };
	inline Factor getSrcFactorA() const { return srcFactorA; };

	inline Factor getDstFactorRGB() const { return dstFactorRGB; };
	inline Factor getDstFactorA() const { return dstFactorA; };

	inline Color getColor() const { return colorConstant; };

	inline bool hasSeparateEquation() const {
		return hasSepEquation;
	}
	inline bool hasSeparateFactor() const {
		return hasSepFactor;
	}
	inline bool hasConstantColor() const {
		return hasColor;
	}

	inline bool operator== (const BlendMode& mode) const {
		return hasSepEquation	== mode.hasSepEquation
			&& hasSepFactor		== mode.hasSepFactor
			&& hasColor			== mode.hasColor
			&& equationRGB		== mode.equationRGB
			&& equationA		== mode.equationA
			&& srcFactorRGB		== mode.srcFactorRGB
			&& dstFactorRGB		== mode.dstFactorRGB
			&& srcFactorA		== mode.srcFactorA
			&& dstFactorA		== mode.dstFactorA
			&& colorConstant	== mode.colorConstant;
	}

private:

	bool hasSepEquation = false;
	bool hasSepFactor = false;
	bool hasColor = false;

	Equation equationRGB;
	Equation equationA;

	Factor srcFactorRGB;
	Factor dstFactorRGB;

	Factor srcFactorA;
	Factor dstFactorA;

	Color colorConstant = ff::Color::White;

	void updateHasColor();
	static bool isConstantFactor(Factor factor);
};

class RenderState {
public:
	RenderState()
		: transform{},
		texture{ Texture::getNullTexture()},
		program{ &ShaderProgram::getDefaultProgram() },
		blend{}
	{

	}

	RenderState(Transform _transform) 
		: transform{ _transform },
		texture{ Texture::getNullTexture() },
		program{ &ShaderProgram::getDefaultProgram() },
		blend{}
	{

	}

	RenderState(TextureRef _texture)
		: transform{ },
		texture{ _texture },
		program{ &ShaderProgram::getDefaultProgram() },
		blend{}
	{

	}

	RenderState(const ShaderProgram& _program)
		: transform{ },
		texture{ Texture::getNullTexture() },
		program{ &_program },
		blend{}
	{

	}
	RenderState(const BlendMode& _blend)
		: transform{ },
		texture{ Texture::getNullTexture() },
		program{ &ShaderProgram::getDefaultProgram() },
		blend{_blend}
	{

	}

	BlendMode blend;
	Transform transform;
	TextureRef texture;
	const ShaderProgram* program;

	//static RenderState Default;

};

}