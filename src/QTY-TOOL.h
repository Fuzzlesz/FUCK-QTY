#pragma once

// The sidebar page for QTY's settings
class QTYTool : public FUCK::ITool
{
public:
	static QTYTool* GetSingleton()
	{
		static QTYTool s;
		return &s;
	}

	const char* Name() const override { return "$QTY_ConfigTitle"_T; }

	void Draw() override;

	// Finishes rebinding a hotkey started from the page
	bool OnAsyncInput(const void* e) override;
};
