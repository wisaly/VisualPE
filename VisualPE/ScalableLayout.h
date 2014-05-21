#pragma once
class ScalableNode;

class ScalableLayout
{
public:
	ScalableLayout(void);
	~ScalableLayout(void);

	void ZoomIn();
	void ZoomOut();
};

class ScalableNode
{
public:
	ScalableNode();

private:
	int level_;
	bool isHor_;
	std::string text_;
	std::string description_;
	COLORREF color_;
	ScalableNode* children_;
};