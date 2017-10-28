//
// Created by Dawid Drozd aka Gelldur on 08.10.17.
//
#include <bee/Cocos2dxBeehive.h>
//////////////////////////////////

#include <cassert>

#include <selene.h>

#include <bee/Node.h>

#include "Label.h"
#include "Rect.h"
#include "Sprite.h"

#include "Graph.h"

namespace Bee
{

Cocos2dxBeehive::Cocos2dxBeehive(const std::vector<std::string>& searchPaths)
		: _graph{new Graph{}}
		, _state{new sel::State{true}}
		, _beehive{_state}
		, _searchPaths{searchPaths}
{
	sel::State& state = *_state;

	std::string luaSearchPaths = ";";
	for (const auto& path : searchPaths)
	{
		assert(path.back() == '/' && "Search path must end with / char");
		luaSearchPaths += path;
		luaSearchPaths += "?.lua;";
	}

	const std::string command = "package.path = package.path .. \"" + luaSearchPaths + "\"";
	if (state(command.c_str()) == false)
	{
		assert(false && "Issue during adding search paths");
	}

	state["Context"].SetClass<Context>();

	state["beehive"].SetObj(*this, "addRelation", &Cocos2dxBeehive::addRelation);
	state["context"].SetObj(*this, "getContext", &Cocos2dxBeehive::getContext);

	state["Node"].SetClass<Node, std::string>("setPosition",
											  &Node::setPosition,
											  "addChild",
											  &Node::addChild,
											  "setSize",
											  &Node::setSize,
											  "setAnchor",
											  &Node::setAnchor,
											  "setScale",
											  &Node::setScale,
											  "setRotation",
											  &Node::setRotation,
											  "setVisible",
											  &Node::setVisible,
											  "setZOrder",
											  &Node::setZOrder,
											  "getNode",
											  &Node::getNode);
	state["Sprite"].SetClass<Sprite, std::string>("sprite",
												  &Sprite::sprite,
												  "setColor",
												  &Sprite::setColor,
												  "setOpacity",
												  &Sprite::setOpacity,
												  "setSize",
												  &Sprite::setSize,
												  "getNode",
												  &Sprite::getNode);
	state["NinePatch"].SetClass<NinePatch, std::string>("sprite",
														&NinePatch::sprite,
														"setColor",
														&NinePatch::setColor,
														"setOpacity",
														&NinePatch::setOpacity,
														"setSize",
														&NinePatch::setSize,
														"getNode",
														&NinePatch::getNode);
	state["Rect"].SetClass<Rect, std::string>(
		"setColor", &Rect::setColor, "setOpacity", &Rect::setOpacity, "getNode", &Rect::getNode);
	state["Label"].SetClass<Label, std::string>("setColor",
												&Label::setColor,
												"setOpacity",
												&Label::setOpacity,
												"setAlign",
												&Label::setAlign,
												"setFont",
												&Label::setFont,
												"setFontSize",
												&Label::setFontSize,
												"setText",
												&Label::setText,
												"getNode",
												&Label::getNode);
}

Cocos2dxBeehive::~Cocos2dxBeehive()
{
	delete _graph;
	_graph = nullptr;
}

cocos2d::CCNode* Cocos2dxBeehive::createView(const std::string& content)
{
	sel::State& state = *_state;
	if (state(content.c_str()) == false)
	{
		return nullptr;
	}
	return extractView();
}

cocos2d::CCNode* Cocos2dxBeehive::findViewById(const std::string& id)
{
	auto found = _graph->findNodeById(id);
	if (found.node == nullptr)
	{
		return nullptr;
	}
	return found.node.get();
}

void Cocos2dxBeehive::addRelation(Node* nodeParent, Node* nodeChild)
{
	assert(nodeParent);
	assert(nodeChild);
	_graph->addRelation(nodeParent, nodeChild);
}

cocos2d::CCNode* Cocos2dxBeehive::createViewFromFile(const std::string& filePath)
{
	sel::State& state = *_state;
	for (const auto& path : _searchPaths)
	{
		if (state.Load(path + filePath))
		{
			return extractView();
		}
	}
	return nullptr;
}

cocos2d::CCNode* Cocos2dxBeehive::extractView()
{
	sel::State& state = *_state;
	auto luaData = state["rootView"];// TODO check safety
	if (luaData.exists() == false)
	{
		CCLOG("Missing rootView");
		return nullptr;
	}
	//Casting lua data if fails will return nullptr
	auto nodeWrapper = static_cast<Bee::Node*>(luaData);
	assert(nodeWrapper);
	return nodeWrapper->node.get();
}

Context Cocos2dxBeehive::getContext()
{
	return Context{this};
}

}
