﻿#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Sample3DSceneRenderer.h"

// Renders Direct3D content on the screen.
namespace SpinningCube
{
	class SpinningCubeMain
	{
	public:
		SpinningCubeMain();
		void CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void Update();
		bool Render();

		void OnWindowSizeChanged();
		void OnDeviceRemoved();

		void OnKeyUp(uint32_t keyCode);

	private:
		// TODO: Replace with your own content renderers.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;

		// Rendering loop timer.
		DX::StepTimer m_timer;
	};
}