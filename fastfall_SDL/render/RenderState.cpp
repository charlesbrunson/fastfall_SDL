#include "RenderState.hpp"

namespace ff {

RenderState RenderState::Default;


BlendMode::BlendMode() :
	equationRGB		{ BlendMode::Equation::ADD },
	equationA		{ BlendMode::Equation::ADD },
	srcFactorRGB{ BlendMode::Factor::SRC_ALPHA },
	srcFactorA	{ BlendMode::Factor::ONE },
	dstFactorRGB{ BlendMode::Factor::ONE_MINUS_SRC_ALPHA },
	dstFactorA	{ BlendMode::Factor::ONE_MINUS_SRC_ALPHA }
{

}

BlendMode::BlendMode(Factor srcFactor, Factor dstFactor, Equation blendEquation) :
	equationRGB	{ blendEquation },
	equationA	{ blendEquation },
	srcFactorRGB{ srcFactor },
	srcFactorA	{ srcFactor },
	dstFactorRGB{ dstFactor },
	dstFactorA	{ dstFactor }
{

}

BlendMode::BlendMode(
	Factor srcFactorColor, Factor dstFactorColor, Equation blendEquationColor,
	Factor srcFactorAlpha, Factor dstFactorAlpha, Equation blendEquationAlpha) :
	equationRGB	{ blendEquationColor },
	equationA	{ blendEquationAlpha },
	srcFactorRGB{ srcFactorColor },
	srcFactorA	{ srcFactorAlpha },
	dstFactorRGB{ dstFactorColor },
	dstFactorA	{ dstFactorAlpha }
{

}




}