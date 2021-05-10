#include "render/RenderState.hpp"

namespace ff {

//RenderState RenderState::Default;


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

void BlendMode::setBlendEquation(Equation equationRGBA) {
	equationRGB = equationRGBA; equationA = equationRGBA;
	hasSepEquation = false;
}
void BlendMode::setBlendEquation(Equation equationColor, Equation equationAlpha) {
	equationRGB = equationColor; equationA = equationAlpha;
	hasSepEquation = equationRGB != equationA;
}

void BlendMode::setSrcFactor(Factor factorRGBA) {
	srcFactorRGB = factorRGBA; srcFactorA = factorRGBA;
	hasSepFactor = dstFactorRGB != dstFactorA;
	updateHasColor();
}
void BlendMode::setSrcFactor(Factor factorColor, Factor factorAlpha) {
	srcFactorRGB = factorColor; srcFactorA = factorAlpha;
	hasSepFactor = srcFactorRGB != srcFactorA || dstFactorRGB != dstFactorA;
	updateHasColor();
}

void BlendMode::setDstFactor(Factor factorRGBA) {
	dstFactorRGB = factorRGBA; dstFactorA = factorRGBA;
	hasSepFactor = srcFactorRGB != srcFactorA;
	updateHasColor();
}
void BlendMode::setDstFactor(Factor factorColor, Factor factorAlpha) {
	dstFactorRGB = factorColor; dstFactorA = factorAlpha;
	hasSepFactor = srcFactorRGB != srcFactorA || dstFactorRGB != dstFactorA;
	updateHasColor();
}
void BlendMode::setColor(ff::Color color) {
	colorConstant = color;
}

void BlendMode::updateHasColor() {
	hasColor = isConstantFactor(srcFactorRGB)
		|| isConstantFactor(dstFactorRGB)
		|| isConstantFactor(srcFactorA)
		|| isConstantFactor(dstFactorA);
}

bool BlendMode::isConstantFactor(Factor factor) {
	return factor == Factor::CONSTANT_COLOR
		|| factor == Factor::ONE_MINUS_CONSTANT_COLOR
		|| factor == Factor::CONSTANT_ALPHA
		|| factor == Factor::ONE_MINUS_CONSTANT_ALPHA;
}


}