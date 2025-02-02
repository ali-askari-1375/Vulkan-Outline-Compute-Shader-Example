
#include <iostream>
#include <vector>
#include <array>
#include <cstdint>
#include <optional>
#include <set>
#include <bitset>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <memory>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <shellscalingapi.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_vulkan.h>

///////////////////////////////////////////////////////////////////////////

using BufferTuple = std::tuple<vk::Buffer, vk::DeviceMemory>;
using ImageTuple = std::tuple<vk::Image, vk::DeviceMemory, vk::ImageView>;
using CommandBufferTuple = std::tuple<vk::CommandBuffer, vk::CommandBuffer, vk::CommandBuffer>;

constexpr std::uint32_t G_PreferredImageCount = 2;
constexpr std::uint32_t G_MaxFramesInFlight = 2;

constexpr std::uint32_t G_BufferTuple_Buffer = 0;
constexpr std::uint32_t G_BufferTuple_DeviceMemory = 1;

constexpr std::uint32_t G_ImageTuple_Image = 0;
constexpr std::uint32_t G_ImageTuple_DeviceMemory = 1;
constexpr std::uint32_t G_ImageTuple_ImageView = 2;

constexpr std::uint32_t G_CommandBufferTuple_Graphics1 = 0;
constexpr std::uint32_t G_CommandBufferTuple_Graphics2 = 1;
constexpr std::uint32_t G_CommandBufferTuple_Compute = 2;

///////////////////////////////////////////////////////////////////////////

void InitWindow();
void ShutdownWindow();

bool Render(bool bClearOnly = false);
bool RenderSingleQueue(std::uint32_t ImageIndex);
bool RenderMultiQueue(std::uint32_t ImageIndex);
void ImGuiRender(vk::CommandBuffer CommandBuffer);

std::uint32_t FindMemoryTypeIndex(std::uint32_t typeFilter, vk::MemoryPropertyFlags Properties);
vk::ShaderModule CreateShader(const std::string &fileName);
BufferTuple CreateBuffer(vk::BufferUsageFlags UsageFlags, vk::DeviceSize ByteSize, void* DataPtr, bool bDeviceLocal = false);
ImageTuple CreateImage(vk::ImageUsageFlags UsageFlags, vk::ImageTiling Tiling, vk::Format Fmt, vk::SampleCountFlagBits SampleCount,
					   std::uint32_t Width, std::uint32_t Height, vk::ImageAspectFlags AspectMask, bool bDeviceLocal = true,
					   vk::ArrayProxy<std::uint32_t> QueueFamilyIndices = {});
vk::CommandBuffer BeginSingleUseGraphicsCommandBuffer();
void EndSingleUseGraphicsCommandBuffer(vk::CommandBuffer);
vk::CommandBuffer BeginSingleUseComputeCommandBuffer();
void EndSingleUseComputeCommandBuffer(vk::CommandBuffer);

void InitVulkan();
void ShutdownVulkan();

void InitPhysicalDevice();
void InitQueueFamilies();
void InitDevice();
void InitQueues();

void InitCommandPools();
void InitCommandBuffers();
void InitSyncObjects();

void InitSurface();

void InitRenderPass();

void InitSwapchain();
void CreateSwapchain();
void DestroySwapchain();
void RecreateSwapchain();
void ShutdownSwapchain();
void PerformInitialOwnershipTransfers();

void InitImGui();
void ShutdownImGui();

void InitGraphicsPipeline();
void ShutdownGraphicsPipeline();

void InitComputePipeline();
void ShutdownComputePipeline();

void InitModel();
void ShutdownModel();

LRESULT CALLBACK WndProc(HWND Hwnd, UINT Msg, WPARAM Wparam, LPARAM Lparam);

///////////////////////////////////////////////////////////////////////////

HINSTANCE G_Hinstance = {};
HWND G_Hwnd = {};

std::uint32_t G_CurrentFrame = 0;

vk::DispatchLoaderDynamic G_DLD = {};

vk::Instance G_VkInstance = {};
vk::PhysicalDevice G_PhysicalDevice = {};
vk::SampleCountFlagBits G_SampleCount = vk::SampleCountFlagBits::e1;

std::optional<std::uint32_t> G_GraphicsQueueFamilyIndex;
std::optional<std::uint32_t> G_PresentQueueFamilyIndex;
std::optional<std::uint32_t> G_ComputeQueueFamilyIndex;

vk::Device G_Device = {};
vk::Queue G_GraphicsQueue;
vk::Queue G_PresentQueue;
vk::Queue G_ComputeQueue;

vk::CommandPool G_GraphicsCommandPoolDynamic = {};
vk::CommandPool G_GraphicsCommandPoolStatic = {};
vk::CommandPool G_ComputeCommandPoolDynamic = {};
vk::CommandPool G_ComputeCommandPoolStatic = {};

std::array<CommandBufferTuple, G_MaxFramesInFlight> G_CommandBuffers = {};

std::array<bool, G_MaxFramesInFlight> G_WaitForFences = {};
std::array<vk::Fence, G_MaxFramesInFlight> G_InFlightFences = {};
std::array<vk::Semaphore, G_MaxFramesInFlight> G_ImageAvailableSemaphores = {};
std::array<vk::Semaphore, G_MaxFramesInFlight> G_OffscreenFinishedSemaphores = {};
std::array<vk::Semaphore, G_MaxFramesInFlight> G_ComputeFinishedSemaphores = {};
std::array<vk::Semaphore, G_MaxFramesInFlight> G_RenderFinishedSemaphores = {};

vk::SurfaceKHR G_Surface = {};
vk::SurfaceFormatKHR G_SurfaceFormat = {};
vk::PresentModeKHR G_SurfacePresentMode = {};
std::uint32_t G_SurfaceImageCount = {};
std::uint32_t G_MaxImageCount = {};
vk::Format G_DepthFormat = {};

vk::RenderPass G_RenderPass = {};

bool G_SwapchainOK = false;
vk::Extent2D G_WindowSize = {};
vk::Extent2D G_SwapchainExtent = {};

vk::SwapchainKHR G_Swapchain = {};

std::vector<ImageTuple> G_OffscreenColorRenderTargets;
std::vector<ImageTuple> G_OffscreenDepthRenderTargets;
std::vector<ImageTuple> G_OffscreenCustomStencilRenderTargets;
std::vector<ImageTuple> G_ColorRenderTargets;

std::vector<vk::Image> G_SwapchainImages = {};
std::vector<vk::ImageView> G_SwapchainImageViews = {};

std::vector<vk::Framebuffer> G_Framebuffers = {};

vk::DescriptorPool G_ImguiDescriptorPool = {};
ImGuiContext *G_ImGuiContext = {};
ImFont *G_ConsolasFont = {};

vk::DescriptorPool G_OffscreenDescriptorPool = {};
vk::DescriptorSetLayout G_OffscreenDescriptorSetLayout = {};
std::vector<vk::DescriptorSet> G_OffscreenDescriptorSets = {};

vk::PipelineLayout G_GraphicsPipelineLayout = {};
vk::Pipeline G_GraphicsPipeline = {};

vk::PipelineLayout G_ComputePipelineLayout = {};
vk::Pipeline G_ComputePipeline = {};

////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int nCmdShow)
{
	::SetProcessDpiAwareness(PROCESS_DPI_UNAWARE);

	G_Hinstance = hInstance;

	InitWindow();
	InitVulkan();

//	ImGui::SetCurrentContext(G_ImGuiContext);
	Render(true);
//	ImGui::SetCurrentContext(nullptr);

	MSG Msg = {};
	bool bContinue = true;

	::ShowWindow(G_Hwnd, SW_SHOW);
	::UpdateWindow(G_Hwnd);

	while(bContinue) {
		for (std::uint32_t i = 0; i < std::uint32_t(1); i++) {
			if (::PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE)) {
				if (Msg.message == WM_QUIT) {
					bContinue = false;
					break;
				}

//				if (Msg.hwnd == G_Hwnd) ImGui::SetCurrentContext(G_ImGuiContext);
				::TranslateMessage(&Msg);
				::DispatchMessage(&Msg);
//				ImGui::SetCurrentContext(nullptr);

			} else {
				break;
			}
		}

//		ImGui::SetCurrentContext(G_ImGuiContext);
		if (!::IsIconic(G_Hwnd)) {
			if (!Render()) {
				RecreateSwapchain();
				Render();
			}
		}
//		ImGui::SetCurrentContext(nullptr);
	}

	ShutdownVulkan();
	ShutdownWindow();

	return 0;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND Hwnd, UINT Msg, WPARAM Wparam, LPARAM Lparam)
{
	const auto InternalWndProc = [](HWND Hwnd, UINT Msg, WPARAM Wparam, LPARAM Lparam) -> LRESULT {
		switch(Msg)
		{
		case WM_SIZE:
		{
			G_SwapchainOK = false;
		}
		break;
		case WM_ERASEBKGND:
		{
			return 1;
		}
		break;
		case WM_DESTROY:
		{
			::PostQuitMessage(0);
		}
		break;
		}

		return ::DefWindowProcA(Hwnd, Msg, Wparam, Lparam);
	};

	const auto Result = InternalWndProc(Hwnd, Msg, Wparam, Lparam);
//	ImGui_ImplWin32_WndProcHandler(Hwnd, Msg, Wparam, Lparam);

	return Result;
}

void InitWindow()
{
	static constexpr DWORD dwStyle = WS_CAPTION | WS_BORDER | WS_MINIMIZEBOX | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX;
	RECT rc = RECT{0, 0, 480, 640};
	::AdjustWindowRectEx(&rc, dwStyle, FALSE, 0);

	const WNDCLASSEXA wcx = WNDCLASSEXA{
		sizeof(WNDCLASSEXA),
		0,
		WndProc,
		0,
		0,
		G_Hinstance,
		LoadIconA(nullptr, IDI_APPLICATION),
		LoadCursorA(nullptr, IDC_ARROW),
		0,
		0,
		"VkOutlineComputeShaderClass",
		LoadIconA(nullptr, IDI_APPLICATION)
	};
	::RegisterClassExA(&wcx);


	G_Hwnd = ::CreateWindowExA(
		0,
		"VkOutlineComputeShaderClass",
		"Vk Outline Compute Shader",
		dwStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		G_Hinstance,
		nullptr
		);
}

void ShutdownWindow()
{
	::UnregisterClassA(
		"VkOutlineComputeShaderClass",
		G_Hinstance
		);
}


bool Render(bool bClearOnly)
{
	if (!G_SwapchainOK) return false;

	if (G_WaitForFences[G_CurrentFrame]) {
		(void)G_Device.waitForFences(1, &G_InFlightFences[G_CurrentFrame], VK_TRUE, std::numeric_limits<std::uint64_t>::max(), G_DLD);
		G_WaitForFences[G_CurrentFrame] = false;
	} else {
		G_GraphicsQueue.waitIdle(G_DLD);
	}
	(void)G_Device.resetFences(1, &G_InFlightFences[G_CurrentFrame], G_DLD);

	std::uint32_t ImageIndex = 0;
	try {
		vk::ResultValue Acquire = G_Device.acquireNextImageKHR(
			G_Swapchain, std::numeric_limits<std::uint64_t>::max(),
			G_ImageAvailableSemaphores[G_CurrentFrame], nullptr,
			G_DLD
			);

		ImageIndex = Acquire.value;
	}
	catch (...) {
		G_SwapchainOK = false;
		return false;
	}


	if (G_GraphicsQueueFamilyIndex.value() == G_ComputeQueueFamilyIndex.value()) {
		if (!RenderSingleQueue(ImageIndex)) return false;
	} else {
		if (!RenderMultiQueue(ImageIndex)) return false;
	}

	const vk::Semaphore WaitSemaphores[] = { G_RenderFinishedSemaphores[G_CurrentFrame]};
	const vk::SwapchainKHR Swapchains[1] = {G_Swapchain};
	const vk::PresentInfoKHR PresentInfo = vk::PresentInfoKHR(1, WaitSemaphores, 1, Swapchains, &ImageIndex);
	try {
		(void)G_PresentQueue.presentKHR(PresentInfo, G_DLD);
	}
	catch (...) {
		G_SwapchainOK = false;
		return false;
	}

	G_CurrentFrame = (G_CurrentFrame + 1) % G_MaxFramesInFlight;

	return true;
}

bool RenderSingleQueue(std::uint32_t ImageIndex)
{
	vk::CommandBuffer CommandBuffer = std::get<G_CommandBufferTuple_Graphics1>(G_CommandBuffers[G_CurrentFrame]);
	CommandBuffer.reset({}, G_DLD);

	const vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
	CommandBuffer.begin(commandBufferBeginInfo, G_DLD);


	vk::ClearColorValue ClearColor;
	std::memcpy(&ClearColor, DirectX::Colors::Khaki.f, sizeof(ClearColor));
	static constexpr vk::ClearColorValue ClearCustomColor(std::array<std::uint32_t, 4>{0,0,0,0});
	static constexpr vk::ClearDepthStencilValue ClearDepth(1.0f, 0);
	const vk::ClearValue ClearValues[3] = {ClearColor, ClearCustomColor, ClearDepth};
	const vk::RenderPassBeginInfo RenderPassBeginInfo = vk::RenderPassBeginInfo(
		G_RenderPass, G_Framebuffers[ImageIndex], vk::Rect2D(vk::Offset2D(0,0), G_SwapchainExtent), 3, ClearValues);

	CommandBuffer.beginRenderPass(&RenderPassBeginInfo, vk::SubpassContents::eInline, G_DLD);

	CommandBuffer.setViewport(0, vk::Viewport{0.0f, 0.0f, float(G_SwapchainExtent.width), float(G_SwapchainExtent.height), 0.0f, 1.0f}, G_DLD);
	CommandBuffer.setScissor(0, vk::Rect2D{vk::Offset2D{0, 0}, vk::Extent2D{G_SwapchainExtent.width, G_SwapchainExtent.height}}, G_DLD);

	CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, G_GraphicsPipeline, G_DLD);
	CommandBuffer.draw(6, 1, 0, 0, G_DLD);

	//ImGuiRender(CommandBuffer);

	CommandBuffer.endRenderPass(G_DLD);

	const vk::ImageMemoryBarrier AcquireImageMemoryBarriers[] = {
		vk::ImageMemoryBarrier(
			vk::AccessFlagBits::eNone,
			vk::AccessFlagBits::eNone,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eGeneral,
			G_GraphicsQueueFamilyIndex.value(),
			G_ComputeQueueFamilyIndex.value(),
			std::get<G_ImageTuple_Image>(G_ColorRenderTargets[ImageIndex]),
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
	};
	CommandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eComputeShader,
		{}, {}, {},
		AcquireImageMemoryBarriers,
		G_DLD
		);

	CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, G_ComputePipeline, G_DLD);
	CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, G_ComputePipelineLayout, 0, G_OffscreenDescriptorSets[ImageIndex], nullptr, G_DLD);

	if (G_SampleCount == vk::SampleCountFlagBits::e1) {
		CommandBuffer.dispatch((G_SwapchainExtent.width / 16) + 1, (G_SwapchainExtent.height / 16) + 1, 1, G_DLD);
	} else {
		CommandBuffer.dispatch((G_SwapchainExtent.width / 16) + 1, (G_SwapchainExtent.height / 16) + 1, static_cast<std::uint32_t>(G_SampleCount), G_DLD);
	}

	const vk::ImageMemoryBarrier ReleaseImageMemoryBarriers[] = {
		vk::ImageMemoryBarrier(
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eNone,
			vk::ImageLayout::eGeneral,
			vk::ImageLayout::eTransferSrcOptimal,
			G_ComputeQueueFamilyIndex.value(),
			G_GraphicsQueueFamilyIndex.value(),
			std::get<G_ImageTuple_Image>(G_ColorRenderTargets[ImageIndex]),
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
	};
	CommandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eColorAttachmentOutput,
		{}, {}, {},
		ReleaseImageMemoryBarriers,
		G_DLD
		);

	const vk::ImageMemoryBarrier AcquireImageMemoryBarriers2[] = {
		vk::ImageMemoryBarrier(
			vk::AccessFlagBits::eNone,
			vk::AccessFlagBits::eTransferWrite,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal,
			vk::QueueFamilyIgnored,
			vk::QueueFamilyIgnored,
			G_SwapchainImages[ImageIndex],
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
	};
	CommandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eColorAttachmentOutput,
		{}, {}, {},
		AcquireImageMemoryBarriers2,
		G_DLD
		);

	if (G_SampleCount != vk::SampleCountFlagBits::e1) {
		const vk::ImageResolve ImageResolve = vk::ImageResolve(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {}, vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {}, vk::Extent3D(G_SwapchainExtent.width, G_SwapchainExtent.height, 1));
		CommandBuffer.resolveImage(std::get<G_ImageTuple_Image>(G_ColorRenderTargets[ImageIndex]), vk::ImageLayout::eTransferSrcOptimal, G_SwapchainImages[ImageIndex], vk::ImageLayout::eTransferDstOptimal, ImageResolve, G_DLD);
	} else {
		const vk::ImageCopy ImageCopy = vk::ImageCopy(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {}, vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {}, vk::Extent3D(G_SwapchainExtent.width, G_SwapchainExtent.height, 1));
		CommandBuffer.copyImage(std::get<G_ImageTuple_Image>(G_ColorRenderTargets[ImageIndex]), vk::ImageLayout::eTransferSrcOptimal, G_SwapchainImages[ImageIndex], vk::ImageLayout::eTransferDstOptimal, ImageCopy, G_DLD);
	}

	const vk::ImageMemoryBarrier ReleaseImageMemoryBarriers2[] = {
		vk::ImageMemoryBarrier(
			vk::AccessFlagBits::eTransferWrite,
			vk::AccessFlagBits::eNone,
			vk::ImageLayout::eTransferDstOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::QueueFamilyIgnored,
			vk::QueueFamilyIgnored,
			G_SwapchainImages[ImageIndex],
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
	};
	CommandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe,
		{}, {}, {},
		ReleaseImageMemoryBarriers2,
		G_DLD
		);

	CommandBuffer.end(G_DLD);

	const vk::Semaphore WaitSemaphores[] = { G_ImageAvailableSemaphores[G_CurrentFrame]};
	const vk::Semaphore SignalSemaphores[] = { G_RenderFinishedSemaphores[G_CurrentFrame]};
	static constexpr vk::PipelineStageFlags WaitStages[] = { vk::PipelineStageFlagBits::eAllCommands };
	const vk::SubmitInfo submitInfo = vk::SubmitInfo(1, WaitSemaphores, WaitStages, 1, &CommandBuffer, 1, SignalSemaphores);
	try {
		G_WaitForFences[G_CurrentFrame] = true;
		G_GraphicsQueue.submit(submitInfo, G_InFlightFences[G_CurrentFrame], G_DLD);
	}
	catch (...) {
		G_WaitForFences[G_CurrentFrame] = false;
		G_SwapchainOK = false;
		return false;
	}

	return true;
}

bool RenderMultiQueue(std::uint32_t ImageIndex)
{
	{
		vk::CommandBuffer CommandBuffer = std::get<G_CommandBufferTuple_Graphics1>(G_CommandBuffers[G_CurrentFrame]);
		CommandBuffer.reset({}, G_DLD);

		const vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
		CommandBuffer.begin(commandBufferBeginInfo, G_DLD);

		const vk::ImageMemoryBarrier AcquireImageMemoryBarriers[] = {
			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eColorAttachmentWrite,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eColorAttachmentOptimal,
				G_ComputeQueueFamilyIndex.value(),
				G_GraphicsQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_OffscreenColorRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eColorAttachmentWrite,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eColorAttachmentOptimal,
				G_ComputeQueueFamilyIndex.value(),
				G_GraphicsQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_OffscreenCustomStencilRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
		};
		CommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, {}, {},
			AcquireImageMemoryBarriers,
			G_DLD
			);


		vk::ClearColorValue ClearColor;
		std::memcpy(&ClearColor, DirectX::Colors::Khaki.f, sizeof(ClearColor));
		static constexpr vk::ClearColorValue ClearCustomColor(std::array<std::uint32_t, 4>{0,0,0,0});
		static constexpr vk::ClearDepthStencilValue ClearDepth(1.0f, 0);
		const vk::ClearValue ClearValues[3] = {ClearColor, ClearCustomColor, ClearDepth};
		const vk::RenderPassBeginInfo RenderPassBeginInfo = vk::RenderPassBeginInfo(
			G_RenderPass, G_Framebuffers[ImageIndex], vk::Rect2D(vk::Offset2D(0,0), G_SwapchainExtent), 3, ClearValues);

		CommandBuffer.beginRenderPass(&RenderPassBeginInfo, vk::SubpassContents::eInline, G_DLD);

		CommandBuffer.setViewport(0, vk::Viewport{0.0f, 0.0f, float(G_SwapchainExtent.width), float(G_SwapchainExtent.height), 0.0f, 1.0f}, G_DLD);
		CommandBuffer.setScissor(0, vk::Rect2D{vk::Offset2D{0, 0}, vk::Extent2D{G_SwapchainExtent.width, G_SwapchainExtent.height}}, G_DLD);

		CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, G_GraphicsPipeline, G_DLD);
		CommandBuffer.draw(6, 1, 0, 0, G_DLD);

		//ImGuiRender(CommandBuffer);

		CommandBuffer.endRenderPass(G_DLD);

		const vk::ImageMemoryBarrier ReleaseImageMemoryBarriers[] = {
			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eColorAttachmentWrite,
				vk::AccessFlagBits::eNone,
				vk::ImageLayout::eGeneral,
				vk::ImageLayout::eGeneral,
				G_GraphicsQueueFamilyIndex.value(),
				G_ComputeQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_OffscreenColorRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eColorAttachmentWrite,
				vk::AccessFlagBits::eNone,
				vk::ImageLayout::eGeneral,
				vk::ImageLayout::eGeneral,
				G_GraphicsQueueFamilyIndex.value(),
				G_ComputeQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_OffscreenCustomStencilRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
		};
		CommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe,
			{}, {}, {},
			ReleaseImageMemoryBarriers,
			G_DLD
			);


		CommandBuffer.end(G_DLD);

		const vk::Semaphore WaitSemaphores[] = { G_ImageAvailableSemaphores[G_CurrentFrame]};
		const vk::Semaphore SignalSemaphores[] = { G_OffscreenFinishedSemaphores[G_CurrentFrame]};
		static constexpr vk::PipelineStageFlags WaitStages[] = { vk::PipelineStageFlagBits::eAllCommands };
		const vk::SubmitInfo submitInfo = vk::SubmitInfo(1, WaitSemaphores, WaitStages, 1, &CommandBuffer, 1, SignalSemaphores);
		try {
			G_WaitForFences[G_CurrentFrame] = true;
			G_GraphicsQueue.submit(submitInfo, nullptr, G_DLD);
		}
		catch (...) {
			G_WaitForFences[G_CurrentFrame] = false;
			G_SwapchainOK = false;
			return false;
		}
	}

	{
		vk::CommandBuffer CommandBuffer = std::get<G_CommandBufferTuple_Compute>(G_CommandBuffers[G_CurrentFrame]);
		CommandBuffer.reset({}, G_DLD);

		const vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
		CommandBuffer.begin(commandBufferBeginInfo, G_DLD);

		const vk::ImageMemoryBarrier AcquireImageMemoryBarriers[] = {
			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eShaderRead,
				vk::ImageLayout::eGeneral,
				vk::ImageLayout::eGeneral,
				G_GraphicsQueueFamilyIndex.value(),
				G_ComputeQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_OffscreenColorRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),

			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eShaderRead,
				vk::ImageLayout::eGeneral,
				vk::ImageLayout::eGeneral,
				G_GraphicsQueueFamilyIndex.value(),
				G_ComputeQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_OffscreenCustomStencilRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),

			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eShaderWrite,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eGeneral,
				G_GraphicsQueueFamilyIndex.value(),
				G_ComputeQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_ColorRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
		};
		CommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader,
			{}, {}, {},
			AcquireImageMemoryBarriers,
			G_DLD
			);

		CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, G_ComputePipeline, G_DLD);
		CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, G_ComputePipelineLayout, 0, G_OffscreenDescriptorSets[ImageIndex], nullptr, G_DLD);

		if (G_SampleCount == vk::SampleCountFlagBits::e1) {
			CommandBuffer.dispatch((G_SwapchainExtent.width / 16) + 1, (G_SwapchainExtent.height / 16) + 1, 1, G_DLD);
		} else {
			CommandBuffer.dispatch((G_SwapchainExtent.width / 16) + 1, (G_SwapchainExtent.height / 16) + 1, static_cast<std::uint32_t>(G_SampleCount), G_DLD);
		}

		const vk::ImageMemoryBarrier ReleaseImageMemoryBarriers[] = {
			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eShaderRead,
				vk::AccessFlagBits::eNone,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eColorAttachmentOptimal,
				G_ComputeQueueFamilyIndex.value(),
				G_GraphicsQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_OffscreenColorRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),

			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eShaderRead,
				vk::AccessFlagBits::eNone,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eColorAttachmentOptimal,
				G_ComputeQueueFamilyIndex.value(),
				G_GraphicsQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_OffscreenCustomStencilRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),

			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eShaderWrite,
				vk::AccessFlagBits::eNone,
				vk::ImageLayout::eGeneral,
				vk::ImageLayout::eTransferSrcOptimal,
				G_ComputeQueueFamilyIndex.value(),
				G_GraphicsQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_ColorRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
		};
		CommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eBottomOfPipe,
			{}, {}, {},
			ReleaseImageMemoryBarriers,
			G_DLD
			);

		CommandBuffer.end(G_DLD);

		const vk::Semaphore WaitSemaphores[] = { G_OffscreenFinishedSemaphores[G_CurrentFrame]};
		const vk::Semaphore SignalSemaphores[] = { G_ComputeFinishedSemaphores[G_CurrentFrame]};
		static constexpr vk::PipelineStageFlags WaitStages[] = { vk::PipelineStageFlagBits::eAllCommands };
		const vk::SubmitInfo submitInfo = vk::SubmitInfo(1, WaitSemaphores, WaitStages, 1, &CommandBuffer, 1, SignalSemaphores);
		try {
			G_WaitForFences[G_CurrentFrame] = true;
			G_ComputeQueue.submit(submitInfo, nullptr, G_DLD);
		}
		catch (...) {
			G_WaitForFences[G_CurrentFrame] = false;
			G_SwapchainOK = false;
			return false;
		}
	}

	{
		vk::CommandBuffer CommandBuffer = std::get<G_CommandBufferTuple_Graphics2>(G_CommandBuffers[G_CurrentFrame]);
		CommandBuffer.reset({}, G_DLD);

		const vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
		CommandBuffer.begin(commandBufferBeginInfo, G_DLD);

		const vk::ImageMemoryBarrier AcquireImageMemoryBarriers[] = {
			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eTransferRead,
				vk::ImageLayout::eGeneral,
				vk::ImageLayout::eTransferSrcOptimal,
				G_ComputeQueueFamilyIndex.value(),
				G_GraphicsQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_ColorRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eTransferWrite,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eTransferDstOptimal,
				vk::QueueFamilyIgnored,
				vk::QueueFamilyIgnored,
				G_SwapchainImages[ImageIndex],
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
		};
		CommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, {}, {},
			AcquireImageMemoryBarriers,
			G_DLD
			);

		if (G_SampleCount != vk::SampleCountFlagBits::e1) {
			const vk::ImageResolve ImageResolve = vk::ImageResolve(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {}, vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {}, vk::Extent3D(G_SwapchainExtent.width, G_SwapchainExtent.height, 1));
			CommandBuffer.resolveImage(std::get<G_ImageTuple_Image>(G_ColorRenderTargets[ImageIndex]), vk::ImageLayout::eTransferSrcOptimal, G_SwapchainImages[ImageIndex], vk::ImageLayout::eTransferDstOptimal, ImageResolve, G_DLD);
		} else {
			const vk::ImageCopy ImageCopy = vk::ImageCopy(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {}, vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {}, vk::Extent3D(G_SwapchainExtent.width, G_SwapchainExtent.height, 1));
			CommandBuffer.copyImage(std::get<G_ImageTuple_Image>(G_ColorRenderTargets[ImageIndex]), vk::ImageLayout::eTransferSrcOptimal, G_SwapchainImages[ImageIndex], vk::ImageLayout::eTransferDstOptimal, ImageCopy, G_DLD);
		}

		const vk::ImageMemoryBarrier ReleaseImageMemoryBarriers[] = {
			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eTransferRead,
				vk::AccessFlagBits::eNone,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eGeneral,
				G_GraphicsQueueFamilyIndex.value(),
				G_ComputeQueueFamilyIndex.value(),
				std::get<G_ImageTuple_Image>(G_ColorRenderTargets[ImageIndex]),
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
			vk::ImageMemoryBarrier(
				vk::AccessFlagBits::eTransferWrite,
				vk::AccessFlagBits::eNone,
				vk::ImageLayout::eTransferDstOptimal,
				vk::ImageLayout::ePresentSrcKHR,
				vk::QueueFamilyIgnored,
				vk::QueueFamilyIgnored,
				G_SwapchainImages[ImageIndex],
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
		};
		CommandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eBottomOfPipe,
			{}, {}, {},
			ReleaseImageMemoryBarriers,
			G_DLD
			);

		CommandBuffer.end(G_DLD);

		const vk::Semaphore WaitSemaphores[] = { G_ComputeFinishedSemaphores[G_CurrentFrame]};
		const vk::Semaphore SignalSemaphores[] = { G_RenderFinishedSemaphores[G_CurrentFrame]};
		static constexpr vk::PipelineStageFlags WaitStages[] = { vk::PipelineStageFlagBits::eAllCommands };
		const vk::SubmitInfo submitInfo = vk::SubmitInfo(1, WaitSemaphores, WaitStages, 1, &CommandBuffer, 1, SignalSemaphores);
		try {
			G_WaitForFences[G_CurrentFrame] = true;
			G_GraphicsQueue.submit(submitInfo, G_InFlightFences[G_CurrentFrame], G_DLD);
		}
		catch (...) {
			G_WaitForFences[G_CurrentFrame] = false;
			G_SwapchainOK = false;
			return false;
		}
	}

	return true;
}

void ImGuiRender(vk::CommandBuffer CommandBuffer)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//	ImGui::Begin("Control", nullptr);
	//	ImGui::PushFont(G_ConsolasFont);
	//	ImGui::PopFont();
	//	ImGui::End();

	static bool bFirst = true;
	if (bFirst) {
		ImGui::SetWindowFocus(nullptr);
		bFirst = false;
	}

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffer);
}


std::uint32_t FindMemoryTypeIndex(std::uint32_t typeFilter, vk::MemoryPropertyFlags Properties)
{
	vk::PhysicalDeviceMemoryProperties MemProperties = G_PhysicalDevice.getMemoryProperties(G_DLD);

	for (std::uint32_t i = 0; i < MemProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && MemProperties.memoryTypes[i].propertyFlags == Properties) {
			return i;
		}
	}

	for (std::uint32_t i = 0; i < MemProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (MemProperties.memoryTypes[i].propertyFlags & Properties) == Properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

vk::ShaderModule CreateShader(const std::string &fileName)
{
	const std::string FilePath = std::string(APP_SOURCE_PATH) + std::string("/shader/") + fileName;
	std::ifstream Ifs = std::ifstream(FilePath, std::ios::binary | std::ios::in);

	if (!Ifs.is_open()) {
		throw std::runtime_error("Could not open shader file");
	}

	const std::vector<char> Buf = std::vector<char>(std::istreambuf_iterator<char>(Ifs), std::istreambuf_iterator<char>());
	Ifs.close();

	const vk::ShaderModuleCreateInfo ShaderModuleCI = vk::ShaderModuleCreateInfo(
		{},
		Buf.size(),
		reinterpret_cast<const std::uint32_t*>(Buf.data())
		);

	return G_Device.createShaderModule(ShaderModuleCI, nullptr, G_DLD);
}

BufferTuple CreateBuffer(vk::BufferUsageFlags UsageFlags, vk::DeviceSize ByteSize, void* DataPtr, bool bDeviceLocal)
{
	const vk::BufferCreateInfo BufferCI = vk::BufferCreateInfo(
		{},
		ByteSize,
		(bDeviceLocal ? (UsageFlags | vk::BufferUsageFlagBits::eTransferSrc) : UsageFlags),
		vk::SharingMode::eExclusive,
		0,
		nullptr
		);

	const vk::Buffer Buffer = G_Device.createBuffer(BufferCI, nullptr, G_DLD);

	const vk::MemoryRequirements MemReqs = G_Device.getBufferMemoryRequirements(Buffer, G_DLD);
	const std::uint32_t MemTypeIndex = FindMemoryTypeIndex(MemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	const vk::MemoryAllocateInfo MemAI = vk::MemoryAllocateInfo(MemReqs.size, MemTypeIndex);

	vk::DeviceMemory BufferMemory = G_Device.allocateMemory(MemAI, nullptr, G_DLD);
	G_Device.bindBufferMemory(Buffer, BufferMemory, 0, G_DLD);

	void* Mapped =G_Device.mapMemory(BufferMemory, 0, vk::WholeSize, vk::MemoryMapFlags(), G_DLD);
	if (DataPtr) {
		std::memcpy(Mapped, DataPtr, ByteSize);
	} else {
		std::memset(Mapped, 0, ByteSize);
	}
	G_Device.unmapMemory(BufferMemory, G_DLD);

	if (bDeviceLocal) {
		const vk::BufferCreateInfo LocalBufferCI = vk::BufferCreateInfo(
			{},
			ByteSize,
			UsageFlags | vk::BufferUsageFlagBits::eTransferDst,
			vk::SharingMode::eExclusive,
			0,
			nullptr
			);

		const vk::Buffer LocalBuffer = G_Device.createBuffer(LocalBufferCI, nullptr, G_DLD);
		const vk::MemoryRequirements LocalMemReqs = G_Device.getBufferMemoryRequirements(LocalBuffer, G_DLD);
		const std::uint32_t LocalMemTypeIndex = FindMemoryTypeIndex(LocalMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		const vk::MemoryAllocateInfo LocalMemAI = vk::MemoryAllocateInfo(LocalMemReqs.size, LocalMemTypeIndex);

		const vk::DeviceMemory LocalBufferMemory = G_Device.allocateMemory(LocalMemAI, nullptr, G_DLD);
		G_Device.bindBufferMemory(LocalBuffer, LocalBufferMemory, 0, G_DLD);

		const vk::CommandBuffer CommandBuffer = BeginSingleUseGraphicsCommandBuffer();
		const vk::BufferCopy BufferCopy = vk::BufferCopy(0, 0, ByteSize);
		CommandBuffer.copyBuffer(Buffer, LocalBuffer, BufferCopy, G_DLD);
		EndSingleUseGraphicsCommandBuffer(CommandBuffer);

		G_Device.freeMemory(BufferMemory, nullptr, G_DLD);
		G_Device.destroyBuffer(Buffer, nullptr, G_DLD);

		return std::make_tuple(LocalBuffer, LocalBufferMemory);
	}

	return std::make_tuple(Buffer, BufferMemory);
}

ImageTuple CreateImage(vk::ImageUsageFlags UsageFlags,
					   vk::ImageTiling Tiling,
					   vk::Format Fmt,
					   vk::SampleCountFlagBits SampleCount,
					   std::uint32_t Width,
					   std::uint32_t Height,
					   vk::ImageAspectFlags AspectMask,
					   bool bDeviceLocal,
					   vk::ArrayProxy<std::uint32_t> QueueFamilyIndices)
{
	const vk::ImageCreateInfo ImageCI = vk::ImageCreateInfo(
		{},
		vk::ImageType::e2D,
		Fmt,
		vk::Extent3D{Width, Height, 1},
		1,
		1,
		SampleCount,
		Tiling,
		UsageFlags,
		vk::SharingMode::eExclusive,
		nullptr, //QueueFamilyIndices,
		vk::ImageLayout::eUndefined
		);

	auto Image = G_Device.createImage(ImageCI, nullptr, G_DLD);
	if (!Image) {
		return {};
	}

	const vk::MemoryRequirements ImageMemReqs = G_Device.getImageMemoryRequirements(Image, G_DLD);
	const vk::MemoryPropertyFlags MemPropFlags = bDeviceLocal ? (vk::MemoryPropertyFlagBits::eDeviceLocal) : (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	const std::uint32_t ImageMemoryTypeIndex = FindMemoryTypeIndex(ImageMemReqs.memoryTypeBits, MemPropFlags);
	const vk::MemoryAllocateInfo ImageMemoryAI = vk::MemoryAllocateInfo{ImageMemReqs.size, ImageMemoryTypeIndex};

	auto ImageMemory = G_Device.allocateMemory(ImageMemoryAI, nullptr, G_DLD);
	if (!ImageMemory) {
		G_Device.destroyImage(Image, nullptr, G_DLD);
		return {};
	}

	G_Device.bindImageMemory(Image, ImageMemory, 0, G_DLD);

	const vk::ImageViewCreateInfo ImageViewCI = vk::ImageViewCreateInfo(
		{},
		Image,
		vk::ImageViewType::e2D,
		Fmt,
		{},
		vk::ImageSubresourceRange{AspectMask, 0, 1, 0, 1}
		);

	auto ImageView = G_Device.createImageView(ImageViewCI, nullptr, G_DLD);
	if (!ImageView) {
		G_Device.freeMemory(ImageMemory, nullptr, G_DLD);
		G_Device.destroyImage(Image, nullptr, G_DLD);
		return {};
	}

	return std::make_tuple(Image, ImageMemory, ImageView);
}

vk::CommandBuffer BeginSingleUseGraphicsCommandBuffer()
{
	const vk::CommandBufferAllocateInfo CommandBufferAI = vk::CommandBufferAllocateInfo(G_GraphicsCommandPoolStatic, vk::CommandBufferLevel::ePrimary, 1);
	vk::CommandBuffer CommandBuffer = G_Device.allocateCommandBuffers(CommandBufferAI, G_DLD)[0];
	const vk::CommandBufferBeginInfo CommandBufferBI = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	CommandBuffer.begin(CommandBufferBI, G_DLD);

	return CommandBuffer;
}

void EndSingleUseGraphicsCommandBuffer(vk::CommandBuffer CommandBuffer)
{
	CommandBuffer.end(G_DLD);
	const vk::SubmitInfo SI = vk::SubmitInfo(0, nullptr, 0, 1, &CommandBuffer, 0, nullptr);
	G_GraphicsQueue.submit(SI, nullptr, G_DLD);
	G_GraphicsQueue.waitIdle(G_DLD);
}

vk::CommandBuffer BeginSingleUseComputeCommandBuffer()
{
	const vk::CommandBufferAllocateInfo CommandBufferAI = vk::CommandBufferAllocateInfo(G_ComputeCommandPoolStatic, vk::CommandBufferLevel::ePrimary, 1);
	vk::CommandBuffer CommandBuffer = G_Device.allocateCommandBuffers(CommandBufferAI, G_DLD)[0];
	const vk::CommandBufferBeginInfo CommandBufferBI = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	CommandBuffer.begin(CommandBufferBI, G_DLD);

	return CommandBuffer;
}

void EndSingleUseComputeCommandBuffer(vk::CommandBuffer CommandBuffer)
{
	CommandBuffer.end(G_DLD);
	const vk::SubmitInfo SI = vk::SubmitInfo(0, nullptr, 0, 1, &CommandBuffer, 0, nullptr);
	G_ComputeQueue.submit(SI, nullptr, G_DLD);
	G_ComputeQueue.waitIdle(G_DLD);
}

void InitVulkan()
{
	G_DLD.init();

	const std::uint32_t VkApiVersion = vk::enumerateInstanceVersion(G_DLD) & (~std::uint32_t(0xFFFU));
	const vk::ApplicationInfo AppInfo = vk::ApplicationInfo(
		"VkOutlineComputeShaderExample",
		VK_MAKE_API_VERSION(0, 1, 0, 0),
		"VkOutlineComputeShaderEngine",
		VK_MAKE_API_VERSION(0, 1, 0, 0),
		VkApiVersion
		);

	static constexpr std::array<const char*, 1> RequiredInstanceLayers = {
		"VK_LAYER_KHRONOS_validation",
	};
	static constexpr std::array<const char*, 3> RequiredInstanceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};
	const std::vector<vk::LayerProperties> SupportedInstanceLayers = vk::enumerateInstanceLayerProperties(G_DLD);
	const std::vector<vk::ExtensionProperties> SupportedInstanceExtensions = vk::enumerateInstanceExtensionProperties(nullptr, G_DLD);

	for (const char* RequiredLayer : RequiredInstanceLayers) {
		if (std::find_if(SupportedInstanceLayers.begin(), SupportedInstanceLayers.end(), [&RequiredLayer](const vk::LayerProperties& LayerProperties) {return std::strcmp(LayerProperties.layerName.data(), RequiredLayer) == 0; }) == SupportedInstanceLayers.end())
			throw std::runtime_error("Vulkan doesn't support required layers");
	}
	for (const char* RequiredExtension : RequiredInstanceExtensions) {
		if (std::find_if(SupportedInstanceExtensions.begin(), SupportedInstanceExtensions.end(), [&RequiredExtension](const vk::ExtensionProperties& ExtensionProperties) {return std::strcmp(ExtensionProperties.extensionName.data(), RequiredExtension) == 0; }) == SupportedInstanceExtensions.end())
			throw std::runtime_error("Vulkan doesn't support required extensions");
	}

	const vk::InstanceCreateInfo VkInstanceCI = vk::InstanceCreateInfo(
		{},
		&AppInfo,
		static_cast<std::uint32_t>(RequiredInstanceLayers.size()), RequiredInstanceLayers.data(),
		static_cast<std::uint32_t>(RequiredInstanceExtensions.size()), RequiredInstanceExtensions.data()
		);
	G_VkInstance = vk::createInstance(VkInstanceCI, nullptr, G_DLD);
	G_DLD.init(G_VkInstance);

	InitPhysicalDevice();
	InitQueueFamilies();
	InitDevice();
	InitQueues();
	InitCommandPools();
	InitCommandBuffers();
	InitSyncObjects();
	InitSurface();
	InitRenderPass();

	InitSwapchain();
//	InitImGui();

	InitGraphicsPipeline();
	InitComputePipeline();
	InitModel();
}

void ShutdownVulkan()
{
	if (G_Device) {
		G_Device.waitIdle(G_DLD);

		ShutdownModel();
		ShutdownComputePipeline();
		ShutdownGraphicsPipeline();

//		ShutdownImGui();
		ShutdownSwapchain();

		if (G_RenderPass) {
			G_Device.destroyRenderPass(G_RenderPass, nullptr, G_DLD);
			G_RenderPass = nullptr;
		}

		if (G_Surface) {
			G_VkInstance.destroySurfaceKHR(G_Surface, nullptr, G_DLD);
			G_Surface = nullptr;
		}

		for (auto& Item : G_InFlightFences) {
			if (Item) {
				G_Device.destroyFence(Item, nullptr, G_DLD);
				Item = nullptr;
			}
		}

		for (auto& Item : G_RenderFinishedSemaphores) {
			if (Item) {
				G_Device.destroySemaphore(Item, nullptr, G_DLD);
				Item = nullptr;
			}
		}

		for (auto& Item : G_ComputeFinishedSemaphores) {
			if (Item) {
				G_Device.destroySemaphore(Item, nullptr, G_DLD);
				Item = nullptr;
			}
		}

		for (auto& Item : G_OffscreenFinishedSemaphores) {
			if (Item) {
				G_Device.destroySemaphore(Item, nullptr, G_DLD);
				Item = nullptr;
			}
		}

		for (auto& Item : G_ImageAvailableSemaphores) {
			if (Item) {
				G_Device.destroySemaphore(Item, nullptr, G_DLD);
				Item = nullptr;
			}
		}

		for (auto& [One, Two, Three] : G_CommandBuffers) {
			if (One) {
				G_Device.freeCommandBuffers(G_GraphicsCommandPoolDynamic, One, G_DLD);
				One = nullptr;
			}
			if (Two) {
				G_Device.freeCommandBuffers(G_GraphicsCommandPoolDynamic, Two, G_DLD);
				Two = nullptr;
			}
			if (Three) {
				G_Device.freeCommandBuffers(G_ComputeCommandPoolDynamic, Three, G_DLD);
				Three = nullptr;
			}
		}

		if (G_ComputeCommandPoolDynamic) {
			G_Device.destroyCommandPool(G_ComputeCommandPoolDynamic, nullptr, G_DLD);
			G_ComputeCommandPoolDynamic = nullptr;
		}

		if (G_ComputeCommandPoolStatic) {
			G_Device.destroyCommandPool(G_ComputeCommandPoolStatic, nullptr, G_DLD);
			G_ComputeCommandPoolStatic = nullptr;
		}


		if (G_GraphicsCommandPoolDynamic) {
			G_Device.destroyCommandPool(G_GraphicsCommandPoolDynamic, nullptr, G_DLD);
			G_GraphicsCommandPoolDynamic = nullptr;
		}

		if (G_GraphicsCommandPoolStatic) {
			G_Device.destroyCommandPool(G_GraphicsCommandPoolStatic, nullptr, G_DLD);
			G_GraphicsCommandPoolStatic = nullptr;
		}

		G_Device.destroy(nullptr, G_DLD);
		G_Device = nullptr;
	}

	if (G_VkInstance) {
		G_VkInstance.destroy(nullptr, G_DLD);
		G_VkInstance = nullptr;
	}
}

void InitPhysicalDevice()
{
	const std::vector<vk::PhysicalDevice> AvailableDevices = G_VkInstance.enumeratePhysicalDevices(G_DLD);
	static constexpr std::array<const char*, 1> RequiredDeviceLayers = {
		"VK_LAYER_KHRONOS_validation",
	};
	static constexpr std::array<const char*, 1> RequiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	std::vector<vk::PhysicalDevice> SuitablePhysicalDevices;

	for (auto PhysDevice : AvailableDevices) {
		bool bAllExtensionsSupported = true;
		bool bAllLayersSupported = true;

		const std::vector<vk::ExtensionProperties> SupportedDeviceExtensions = PhysDevice.enumerateDeviceExtensionProperties(nullptr, G_DLD);
		const std::vector<vk::LayerProperties> SupportedDeviceLayers = PhysDevice.enumerateDeviceLayerProperties(G_DLD);

		for (const char* RequiredLayer : RequiredDeviceLayers) {
			if (std::find_if(SupportedDeviceLayers.begin(), SupportedDeviceLayers.end(), [&RequiredLayer](const vk::LayerProperties& ExtensionProperties) {return std::strcmp(ExtensionProperties.layerName.data(), RequiredLayer) == 0; }) == SupportedDeviceLayers.end()) {
				bAllLayersSupported = false;
				break;
			}
		}
		for (const char* RequiredExtension : RequiredDeviceExtensions) {
			if (std::find_if(SupportedDeviceExtensions.begin(), SupportedDeviceExtensions.end(), [&RequiredExtension](const vk::ExtensionProperties& ExtensionProperties) {return std::strcmp(ExtensionProperties.extensionName.data(), RequiredExtension) == 0; }) == SupportedDeviceExtensions.end()) {
				bAllExtensionsSupported = false;
				break;
			}
		}
		if (bAllLayersSupported && bAllExtensionsSupported) {
			SuitablePhysicalDevices.push_back(PhysDevice);
		}
	}

	if (!SuitablePhysicalDevices.empty()) {
		for (auto Item : SuitablePhysicalDevices) {
			auto Props = Item.getProperties(G_DLD);
			if (Props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
				G_PhysicalDevice = Item;
				break;
			}
		}

		if (!G_PhysicalDevice)
			G_PhysicalDevice = SuitablePhysicalDevices[0];
	}

	if (!G_PhysicalDevice)
		throw std::runtime_error("Could not find any suitable physical device");

	const vk::PhysicalDeviceProperties PhysDeviceProps = G_PhysicalDevice.getProperties(G_DLD);

	const vk::SampleCountFlags SampleCountFlags = PhysDeviceProps.limits.framebufferColorSampleCounts & PhysDeviceProps.limits.framebufferDepthSampleCounts;
//	if (SampleCountFlags & vk::SampleCountFlagBits::e8) { G_SampleCount = vk::SampleCountFlagBits::e8; }
	if (SampleCountFlags & vk::SampleCountFlagBits::e4) { G_SampleCount = vk::SampleCountFlagBits::e4; }
	else if (SampleCountFlags & vk::SampleCountFlagBits::e2) { G_SampleCount = vk::SampleCountFlagBits::e2; }
	else { G_SampleCount = vk::SampleCountFlagBits::e1; }

}

void InitQueueFamilies()
{
	G_GraphicsQueueFamilyIndex.reset();
	G_PresentQueueFamilyIndex.reset();
	G_ComputeQueueFamilyIndex.reset();

	const std::vector<vk::QueueFamilyProperties> QueueFamilies = G_PhysicalDevice.getQueueFamilyProperties(G_DLD);

	std::vector<vk::Bool32> SupportsPresent = std::vector<vk::Bool32>(QueueFamilies.size(), vk::False);
	for (std::uint32_t i = 0; i < QueueFamilies.size(); i++) {
		SupportsPresent[i] = G_PhysicalDevice.getWin32PresentationSupportKHR(i, G_DLD);
	}

	for (std::uint32_t i = 0; i < QueueFamilies.size(); i++) {
		if (QueueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			if (SupportsPresent[i] == vk::True) {
				G_GraphicsQueueFamilyIndex = static_cast<std::uint32_t>(i);
				G_PresentQueueFamilyIndex = static_cast<std::uint32_t>(i);
				break;
			}
			if (!G_GraphicsQueueFamilyIndex.has_value()) {
				G_GraphicsQueueFamilyIndex = static_cast<std::uint32_t>(i);
			}
		}
	}
	if (!G_PresentQueueFamilyIndex.has_value()) {
		for (std::uint32_t i = 0; i < QueueFamilies.size(); i++) {
			if (SupportsPresent[i] == vk::True) {
				G_PresentQueueFamilyIndex = static_cast<std::uint32_t>(i);
				break;
			}
		}
	}

	for (std::uint32_t i = 0; i < QueueFamilies.size(); i++) {
		if (QueueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) {
			if (!(QueueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
				G_ComputeQueueFamilyIndex = static_cast<std::uint32_t>(i);
				break;
			}
		}
	}

	if (!G_ComputeQueueFamilyIndex.has_value()) {
		for (std::uint32_t i = 0; i < QueueFamilies.size(); i++) {
			if (QueueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) {
				G_ComputeQueueFamilyIndex = static_cast<std::uint32_t>(i);
				break;
			}
		}
	}

	if (!G_GraphicsQueueFamilyIndex.has_value() ||
		!G_PresentQueueFamilyIndex.has_value() ||
		!G_ComputeQueueFamilyIndex.has_value()) {
		throw std::runtime_error("Failed to find all required queue families");
	}
}

void InitDevice()
{
	std::set<std::uint32_t> UniqueIndices;
	UniqueIndices.insert(G_GraphicsQueueFamilyIndex.value());
	UniqueIndices.insert(G_PresentQueueFamilyIndex.value());
	UniqueIndices.insert(G_ComputeQueueFamilyIndex.value());

	std::vector<vk::DeviceQueueCreateInfo> QueueCIs;
	const float QueuePriority = 1.0f;
	for (std::uint32_t QueueFamilyIndex : UniqueIndices) {
		QueueCIs.push_back(
			vk::DeviceQueueCreateInfo(
				{},
				QueueFamilyIndex,
				1,
				&QueuePriority
				)
			);
	}

	static constexpr std::array<const char*, 1> EnabledLayers = {
		"VK_LAYER_KHRONOS_validation",
	};
	static constexpr std::array<const char*, 1> EnabledExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	vk::PhysicalDeviceFeatures EnabledFeatures = vk::PhysicalDeviceFeatures{};
	EnabledFeatures.setFillModeNonSolid(vk::True);
	EnabledFeatures.setShaderStorageImageMultisample(vk::True);
	//EnabledFeatures.setSampleRateShading(vk::True);

	const vk::DeviceCreateInfo DeviceCI = vk::DeviceCreateInfo(
		{},
		static_cast<std::uint32_t>(QueueCIs.size()), QueueCIs.data(),
		static_cast<std::uint32_t>(EnabledLayers.size()), EnabledLayers.data(),
		static_cast<std::uint32_t>(EnabledExtensions.size()), EnabledExtensions.data(),
		&EnabledFeatures
		);

	G_Device = G_PhysicalDevice.createDevice(DeviceCI, nullptr, G_DLD);
	if (!G_Device)
		throw std::runtime_error("Failed to create logical device");
	G_DLD.init(G_Device);
}

void InitQueues()
{
	G_GraphicsQueue = G_Device.getQueue(G_GraphicsQueueFamilyIndex.value(), 0, G_DLD);
	G_PresentQueue = G_Device.getQueue(G_PresentQueueFamilyIndex.value(), 0, G_DLD);
	G_ComputeQueue = G_Device.getQueue(G_ComputeQueueFamilyIndex.value(), 0, G_DLD);
}

void InitCommandPools()
{
	{
		const vk::CommandPoolCreateInfo DynamicCommandPoolCI = vk::CommandPoolCreateInfo(
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			G_GraphicsQueueFamilyIndex.value()
		);
		G_GraphicsCommandPoolDynamic = G_Device.createCommandPool(DynamicCommandPoolCI, nullptr, G_DLD);

		const vk::CommandPoolCreateInfo StaticCommandPoolCI = vk::CommandPoolCreateInfo(
			{},
			G_GraphicsQueueFamilyIndex.value()
		);
		G_GraphicsCommandPoolStatic = G_Device.createCommandPool(StaticCommandPoolCI, nullptr, G_DLD);
	}
	{
	 const vk::CommandPoolCreateInfo DynamicCommandPoolCI = vk::CommandPoolCreateInfo(
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			G_ComputeQueueFamilyIndex.value()
		);
		G_ComputeCommandPoolDynamic = G_Device.createCommandPool(DynamicCommandPoolCI, nullptr, G_DLD);

		const vk::CommandPoolCreateInfo StaticCommandPoolCI = vk::CommandPoolCreateInfo{
																						{},
			G_ComputeQueueFamilyIndex.value()
		};
		G_ComputeCommandPoolStatic = G_Device.createCommandPool(StaticCommandPoolCI, nullptr, G_DLD);
	}
}

void InitCommandBuffers()
{
	const vk::CommandBufferAllocateInfo GraphicsCommandBufferAI = vk::CommandBufferAllocateInfo(
		G_GraphicsCommandPoolDynamic,
		vk::CommandBufferLevel::ePrimary,
		G_MaxFramesInFlight * 2
		);

	const vk::CommandBufferAllocateInfo ComputeCommandBufferAI = vk::CommandBufferAllocateInfo(
		G_ComputeCommandPoolDynamic,
		vk::CommandBufferLevel::ePrimary,
		G_MaxFramesInFlight
		);

	const std::vector<vk::CommandBuffer> GraphicsCmdBuffers = G_Device.allocateCommandBuffers(GraphicsCommandBufferAI, G_DLD);
	const std::vector<vk::CommandBuffer> ComputeCmdBuffers = G_Device.allocateCommandBuffers(ComputeCommandBufferAI, G_DLD);

	for (std::uint32_t i = 0; i < G_MaxFramesInFlight; i++) {
		std::get<G_CommandBufferTuple_Graphics1>(G_CommandBuffers[i]) = GraphicsCmdBuffers[i];
		std::get<G_CommandBufferTuple_Graphics2>(G_CommandBuffers[i]) = GraphicsCmdBuffers[i + G_MaxFramesInFlight];
		std::get<G_CommandBufferTuple_Compute>(G_CommandBuffers[i]) = ComputeCmdBuffers[i];
	}
}

void InitSyncObjects()
{
	const vk::SemaphoreCreateInfo SemaphoreCI = {};
	const vk::FenceCreateInfo FenceCI = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);

	for (std::size_t i = 0; i < G_MaxFramesInFlight; i++) {
		G_ImageAvailableSemaphores[i] = G_Device.createSemaphore(SemaphoreCI, nullptr, G_DLD);
		G_OffscreenFinishedSemaphores[i] = G_Device.createSemaphore(SemaphoreCI, nullptr, G_DLD);
		G_ComputeFinishedSemaphores[i] = G_Device.createSemaphore(SemaphoreCI, nullptr, G_DLD);
		G_RenderFinishedSemaphores[i] = G_Device.createSemaphore(SemaphoreCI, nullptr, G_DLD);
		G_InFlightFences[i] = G_Device.createFence(FenceCI, nullptr, G_DLD);
		G_WaitForFences[i] = true;
	}
}

void InitSurface()
{
	const vk::Win32SurfaceCreateInfoKHR SurfaceCI = vk::Win32SurfaceCreateInfoKHR({}, G_Hinstance, G_Hwnd, {});

	G_Surface = G_VkInstance.createWin32SurfaceKHR(SurfaceCI, nullptr, G_DLD);
	if (!G_Device)
		throw std::runtime_error("Failed to create win32 surface");


	const std::vector<vk::SurfaceFormatKHR> SurfaceFormats = G_PhysicalDevice.getSurfaceFormatsKHR(G_Surface, G_DLD);
	if (SurfaceFormats.empty())
		throw std::runtime_error("The surface doesn't support any format");

	G_SurfaceFormat = SurfaceFormats[0];
	for (auto& SurfaceFormat : SurfaceFormats) {
		if (SurfaceFormat.format == vk::Format::eB8G8R8A8Unorm && SurfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			G_SurfaceFormat = SurfaceFormat;
			break;
		}
	}

	const std::vector<vk::PresentModeKHR> PresentModes = G_PhysicalDevice.getSurfacePresentModesKHR(G_Surface, G_DLD);
	if (PresentModes.empty())
		throw std::runtime_error("The surface doesn't support any present mode");

	G_SurfacePresentMode = PresentModes[0];
	if (G_SurfacePresentMode != vk::PresentModeKHR::eFifo) {
		for (auto& PresentMode : PresentModes) {
			if (PresentMode == vk::PresentModeKHR::eFifo) {
				G_SurfacePresentMode = PresentMode;
				break;
			}
		}
	}
	if (G_SurfacePresentMode != vk::PresentModeKHR::eMailbox && G_SurfacePresentMode != vk::PresentModeKHR::eFifo) {
		for (auto& PresentMode : PresentModes) {
			if (PresentMode == vk::PresentModeKHR::eMailbox) {
				G_SurfacePresentMode = PresentMode;
				break;
			}
		}
	}

	vk::SurfaceCapabilitiesKHR SurfaceCapabilities = G_PhysicalDevice.getSurfaceCapabilitiesKHR(G_Surface, G_DLD);
	G_SurfaceImageCount = std::min(
		SurfaceCapabilities.maxImageCount,
		std::max(SurfaceCapabilities.minImageCount, G_PreferredImageCount)
		);
	G_MaxImageCount = SurfaceCapabilities.maxImageCount;

	static constexpr std::array<vk::Format, 5> CandidateDepthFormats = std::array<vk::Format, 5>(
		{
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD24UnormS8Uint,
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD32Sfloat,
			vk::Format::eD16Unorm,
		}
		);

	G_DepthFormat = vk::Format::eUndefined;
	for (auto& Fmt : CandidateDepthFormats) {
		const vk::FormatProperties FormatProps = G_PhysicalDevice.getFormatProperties(Fmt, G_DLD);
		if ((FormatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) == vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
			G_DepthFormat = Fmt;
			break;
		}
	}
	if (G_DepthFormat == vk::Format::eUndefined)
		throw std::runtime_error("Failed to find proper depth format");
}

void InitRenderPass()
{
	const std::array<vk::AttachmentDescription, 3> Attachments = std::array<vk::AttachmentDescription, 3>(
		{
			vk::AttachmentDescription(
				{},
				G_SurfaceFormat.format,
				G_SampleCount,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eGeneral
				),
			vk::AttachmentDescription(
				{},
				vk::Format::eR8G8B8A8Uint,
				G_SampleCount,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eGeneral
				),
			vk::AttachmentDescription(
				{},
				G_DepthFormat,
				G_SampleCount,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eDontCare,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal
				),
		 }
		);

	static constexpr std::array<vk::AttachmentReference, 2> ColorAttachmentRefs = {
		vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal),
		vk::AttachmentReference(1, vk::ImageLayout::eColorAttachmentOptimal),
	};
	static constexpr std::array<vk::AttachmentReference, 1> DepthAttachmentRefs = {
		vk::AttachmentReference(2, vk::ImageLayout::eDepthStencilAttachmentOptimal),
	};

	static constexpr vk::SubpassDescription Subpass = vk::SubpassDescription(
		{},
		vk::PipelineBindPoint::eGraphics,
		0,
		nullptr,
		2,
		&ColorAttachmentRefs[0],
		nullptr,
		&DepthAttachmentRefs[0],
		0,
		nullptr
		);

	std::vector<vk::SubpassDependency> Dependencies;
	if (G_ComputeQueueFamilyIndex.value() == G_GraphicsQueueFamilyIndex.value()) {
		Dependencies.push_back(
			vk::SubpassDependency(
				vk::SubpassExternal,
				0,
				vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eColorAttachmentWrite
				)
			);
		Dependencies.push_back(
			vk::SubpassDependency(
				0,
				vk::SubpassExternal,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eComputeShader,
				vk::AccessFlagBits::eColorAttachmentWrite,
				vk::AccessFlagBits::eShaderRead
				)
			);

	} else {
		Dependencies.push_back(
			vk::SubpassDependency(
				vk::SubpassExternal,
				0,
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eNone
				)
			);
		Dependencies.push_back(
			vk::SubpassDependency(
				0,
				vk::SubpassExternal,
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				vk::AccessFlagBits::eNone,
				vk::AccessFlagBits::eNone
				)
			);
	}

	const vk::RenderPassCreateInfo RenderPassCI = vk::RenderPassCreateInfo(
		{},
		static_cast<std::uint32_t>(Attachments.size()),
		Attachments.data(),
		1,
		&Subpass,
		static_cast<std::uint32_t>(Dependencies.size()),
		Dependencies.data()
		);

	G_RenderPass = G_Device.createRenderPass(RenderPassCI, nullptr, G_DLD);
	if (!G_RenderPass)
		throw std::runtime_error("Failed to create renderpass");
}



void InitSwapchain()
{
	const vk::DescriptorPoolSize PoolSizes[3] = {
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, G_MaxImageCount),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, G_MaxImageCount),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, G_MaxImageCount),
	};
	const vk::DescriptorPoolCreateInfo DescriptorPoolCI = vk::DescriptorPoolCreateInfo({}, G_MaxImageCount, PoolSizes);
	G_OffscreenDescriptorPool = G_Device.createDescriptorPool(DescriptorPoolCI, nullptr, G_DLD);

	static constexpr vk::DescriptorSetLayoutBinding LayoutBinding[3] = {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute),
		vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute),
	};
	static constexpr vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCI = vk::DescriptorSetLayoutCreateInfo({}, 3, LayoutBinding);
	G_OffscreenDescriptorSetLayout = G_Device.createDescriptorSetLayout(DescriptorSetLayoutCI, nullptr, G_DLD);

	G_OffscreenDescriptorSets.resize(G_MaxImageCount);
	for (std::uint32_t i = 0; i < G_MaxImageCount; i++) {
		vk::DescriptorSetAllocateInfo DescriptorSetAI = vk::DescriptorSetAllocateInfo(G_OffscreenDescriptorPool, 1, &G_OffscreenDescriptorSetLayout);
		G_OffscreenDescriptorSets[i] = G_Device.allocateDescriptorSets(DescriptorSetAI, G_DLD)[0];
	}

	CreateSwapchain();
}

void CreateSwapchain()
{
	if (G_SwapchainOK) {
		DestroySwapchain();
	}

	RECT rc = {};
	::GetClientRect(G_Hwnd, &rc);
	G_WindowSize = vk::Extent2D{static_cast<std::uint32_t>(rc.right - rc.left), static_cast<std::uint32_t>(rc.bottom - rc.top)};

	try {

		const vk::SurfaceCapabilitiesKHR SurfaceCapabilities = G_PhysicalDevice.getSurfaceCapabilitiesKHR(G_Surface, G_DLD);

		G_SwapchainExtent = SurfaceCapabilities.currentExtent;
		if (G_SwapchainExtent.width == std::numeric_limits<std::uint32_t>::max()) {
			G_SwapchainExtent.width = G_WindowSize.width;
			G_SwapchainExtent.width = std::min(SurfaceCapabilities.maxImageExtent.width,std::max(SurfaceCapabilities.minImageExtent.width, G_SwapchainExtent.width));
		}
		if (G_SwapchainExtent.height == std::numeric_limits<std::uint32_t>::max()) {
			G_SwapchainExtent.height = G_WindowSize.height;
			G_SwapchainExtent.height = std::min(SurfaceCapabilities.maxImageExtent.height,std::max(SurfaceCapabilities.minImageExtent.height, G_SwapchainExtent.height));
		}
		if (G_SwapchainExtent.width == 0 || G_SwapchainExtent.height == 0) return;

		if (G_GraphicsQueueFamilyIndex.value() != G_PresentQueueFamilyIndex.value()) {
			const std::uint32_t QueueFamilyIndices[] = { G_GraphicsQueueFamilyIndex.value(), G_PresentQueueFamilyIndex.value()};
			const vk::SwapchainCreateInfoKHR swapchainCI = vk::SwapchainCreateInfoKHR(
				{},
				G_Surface,
				G_SurfaceImageCount,
				G_SurfaceFormat.format,
				G_SurfaceFormat.colorSpace,
				G_SwapchainExtent,
				1,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
				vk::SharingMode::eConcurrent,
				2,
				QueueFamilyIndices,
				SurfaceCapabilities.currentTransform,
				vk::CompositeAlphaFlagBitsKHR::eOpaque,
				G_SurfacePresentMode,
				vk::True
				);
			G_Swapchain = G_Device.createSwapchainKHR(swapchainCI, nullptr, G_DLD);
		}
		else {
			const vk::SwapchainCreateInfoKHR swapchainCI = vk::SwapchainCreateInfoKHR(
				{},
				G_Surface,
				G_SurfaceImageCount,
				G_SurfaceFormat.format,
				G_SurfaceFormat.colorSpace,
				G_SwapchainExtent,
				1,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
				vk::SharingMode::eExclusive,
				{},
				{},
				SurfaceCapabilities.currentTransform,
				vk::CompositeAlphaFlagBitsKHR::eOpaque,
				G_SurfacePresentMode,
				vk::True
				);
			G_Swapchain = G_Device.createSwapchainKHR(swapchainCI, nullptr, G_DLD);
		}

		if (!G_Swapchain)return;
		G_SwapchainOK = true;

	} catch(...) {
		return;
	}

	G_SwapchainImages = G_Device.getSwapchainImagesKHR(G_Swapchain, G_DLD);
	if (G_SwapchainImages.empty()) {
		DestroySwapchain();
		return;
	}

	const std::size_t NumImages = G_SwapchainImages.size();
	G_SwapchainImageViews.resize(NumImages);
	std::fill(G_SwapchainImageViews.begin(), G_SwapchainImageViews.end(), nullptr);

	for (std::size_t i = 0; i < NumImages; ++i) {
		const vk::ImageViewCreateInfo ImageViewCI = vk::ImageViewCreateInfo(
			{},
			G_SwapchainImages[i],
			vk::ImageViewType::e2D,
			G_SurfaceFormat.format,
			vk::ComponentMapping(),
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
			);

		G_SwapchainImageViews[i] = G_Device.createImageView(ImageViewCI, nullptr, G_DLD);
		if (!G_SwapchainImageViews[i]) {
			DestroySwapchain();
			return;
		}
	}

	std::vector<std::uint32_t> QueueFamilyIndices = {G_GraphicsQueueFamilyIndex.value(), G_ComputeQueueFamilyIndex.value()};
	//if (G_GraphicsQueueFamilyIndex.value() == G_ComputeQueueFamilyIndex.value()) QueueFamilyIndices.clear();


	G_OffscreenColorRenderTargets.resize(NumImages);
	std::fill(G_OffscreenColorRenderTargets.begin(), G_OffscreenColorRenderTargets.end(), std::make_tuple<vk::Image, vk::DeviceMemory, vk::ImageView>({}, {}, {}));
	for (std::size_t i = 0; i < NumImages; i++) {
		G_OffscreenColorRenderTargets[i] = CreateImage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage, vk::ImageTiling::eOptimal, G_SurfaceFormat.format, G_SampleCount,
													   G_SwapchainExtent.width, G_SwapchainExtent.height, vk::ImageAspectFlagBits::eColor, true,
													   QueueFamilyIndices);
		if (!std::get<G_ImageTuple_Image>(G_OffscreenColorRenderTargets[i])) {
			DestroySwapchain();
			return;
		}

	}

	G_OffscreenDepthRenderTargets.resize(NumImages);
	std::fill(G_OffscreenDepthRenderTargets.begin(), G_OffscreenDepthRenderTargets.end(), std::make_tuple<vk::Image, vk::DeviceMemory, vk::ImageView>({}, {}, {}));
	for (std::size_t i = 0; i < NumImages; i++) {
		G_OffscreenDepthRenderTargets[i] = CreateImage(vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageTiling::eOptimal, G_DepthFormat, G_SampleCount,
									  G_SwapchainExtent.width, G_SwapchainExtent.height, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, true);
		if (!std::get<G_ImageTuple_Image>(G_OffscreenDepthRenderTargets[i])) {
			DestroySwapchain();
			return;
		}
	}

	G_OffscreenCustomStencilRenderTargets.resize(NumImages);
	std::fill(G_OffscreenCustomStencilRenderTargets.begin(), G_OffscreenCustomStencilRenderTargets.end(), std::make_tuple<vk::Image, vk::DeviceMemory, vk::ImageView>({}, {}, {}));
	for (std::size_t i = 0; i < NumImages; i++) {
		G_OffscreenCustomStencilRenderTargets[i] = CreateImage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage, vk::ImageTiling::eOptimal, vk::Format::eR8G8B8A8Uint, G_SampleCount,
									  G_SwapchainExtent.width, G_SwapchainExtent.height, vk::ImageAspectFlagBits::eColor, true,
															   QueueFamilyIndices);
		if (!std::get<G_ImageTuple_Image>(G_OffscreenCustomStencilRenderTargets[i])) {
			DestroySwapchain();
			return;
		}
	}

	G_ColorRenderTargets.resize(NumImages);
	std::fill(G_ColorRenderTargets.begin(), G_ColorRenderTargets.end(), std::make_tuple<vk::Image, vk::DeviceMemory, vk::ImageView>({}, {}, {}));
	for (std::size_t i = 0; i < NumImages; i++) {
		G_ColorRenderTargets[i] = CreateImage(vk::ImageUsageFlagBits::eStorage |  vk::ImageUsageFlagBits::eTransferSrc, vk::ImageTiling::eOptimal, G_SurfaceFormat.format, G_SampleCount,
												G_SwapchainExtent.width, G_SwapchainExtent.height, vk::ImageAspectFlagBits::eColor, true,
												QueueFamilyIndices);
		if (!std::get<G_ImageTuple_Image>(G_ColorRenderTargets[i])) {
			DestroySwapchain();
			return;
		}
	}

	for (std::size_t i = 0; i < NumImages; i++) {
		const vk::DescriptorImageInfo DescriptorII[3] = {
			vk::DescriptorImageInfo(nullptr, std::get<G_ImageTuple_ImageView>(G_OffscreenColorRenderTargets[i]), vk::ImageLayout::eGeneral),
			vk::DescriptorImageInfo(nullptr, std::get<G_ImageTuple_ImageView>(G_OffscreenCustomStencilRenderTargets[i]), vk::ImageLayout::eGeneral),
			vk::DescriptorImageInfo(nullptr, std::get<G_ImageTuple_ImageView>(G_ColorRenderTargets[i]), vk::ImageLayout::eGeneral),
		};
		const vk::WriteDescriptorSet WriteDS = vk::WriteDescriptorSet(G_OffscreenDescriptorSets[i], 0, 0, 3, vk::DescriptorType::eStorageImage, DescriptorII);
		G_Device.updateDescriptorSets(WriteDS, nullptr, G_DLD);
	}


	G_Framebuffers.resize(NumImages);
	std::fill(G_Framebuffers.begin(), G_Framebuffers.end(), nullptr);
	for (std::size_t i = 0; i < NumImages; ++i) {
		const std::vector<vk::ImageView> Attachments = std::vector<vk::ImageView>(
			{
			 std::get<G_ImageTuple_ImageView>(G_OffscreenColorRenderTargets[i]),
			 std::get<G_ImageTuple_ImageView>(G_OffscreenCustomStencilRenderTargets[i]),
			 std::get<G_ImageTuple_ImageView>(G_OffscreenDepthRenderTargets[i])
			});

		const vk::FramebufferCreateInfo FramebufferCI = vk::FramebufferCreateInfo(
			{},
			G_RenderPass,
			static_cast<std::uint32_t>(Attachments.size()),
			Attachments.data(),
			G_SwapchainExtent.width,
			G_SwapchainExtent.height,
			1
			);

		G_Framebuffers[i] = G_Device.createFramebuffer(FramebufferCI, nullptr, G_DLD);
		if (!G_Framebuffers[i]) {
			DestroySwapchain();
			return;
		}
	}

	PerformInitialOwnershipTransfers();

}

void DestroySwapchain()
{
	G_SwapchainOK = false;

	if (G_Device) {
		G_Device.waitIdle(G_DLD);

		for(auto& Item : G_Framebuffers) {
			if (Item) {
				G_Device.destroyFramebuffer(Item, nullptr, G_DLD);
				Item = nullptr;
			}
		}
		G_Framebuffers.clear();

		for(auto& [Image, ImageMemory, ImageView] : G_ColorRenderTargets) {
			if (ImageView) {
				G_Device.destroyImageView(ImageView, nullptr, G_DLD);
				ImageView = nullptr;
			}
			if (ImageMemory) {
				G_Device.freeMemory(ImageMemory, nullptr, G_DLD);
				ImageMemory = nullptr;
			}
			if (Image) {
				G_Device.destroyImage(Image, nullptr, G_DLD);
				Image = nullptr;
			}
		}
		G_ColorRenderTargets.clear();

		for(auto& [Image, ImageMemory, ImageView] : G_OffscreenCustomStencilRenderTargets) {
			if (ImageView) {
				G_Device.destroyImageView(ImageView, nullptr, G_DLD);
				ImageView = nullptr;
			}
			if (ImageMemory) {
				G_Device.freeMemory(ImageMemory, nullptr, G_DLD);
				ImageMemory = nullptr;
			}
			if (Image) {
				G_Device.destroyImage(Image, nullptr, G_DLD);
				Image = nullptr;
			}
		}
		G_OffscreenCustomStencilRenderTargets.clear();


		for(auto& [Image, ImageMemory, ImageView] : G_OffscreenDepthRenderTargets) {
			if (ImageView) {
				G_Device.destroyImageView(ImageView, nullptr, G_DLD);
				ImageView = nullptr;
			}
			if (ImageMemory) {
				G_Device.freeMemory(ImageMemory, nullptr, G_DLD);
				ImageMemory = nullptr;
			}
			if (Image) {
				G_Device.destroyImage(Image, nullptr, G_DLD);
				Image = nullptr;
			}
		}
		G_OffscreenDepthRenderTargets.clear();

		for(auto& [Image, ImageMemory, ImageView] : G_OffscreenColorRenderTargets) {
			if (ImageView) {
				G_Device.destroyImageView(ImageView, nullptr, G_DLD);
				ImageView = nullptr;
			}
			if (ImageMemory) {
				G_Device.freeMemory(ImageMemory, nullptr, G_DLD);
				ImageMemory = nullptr;
			}
			if (Image) {
				G_Device.destroyImage(Image, nullptr, G_DLD);
				Image = nullptr;
			}
		}
		G_OffscreenColorRenderTargets.clear();

		for(auto& Item : G_SwapchainImageViews) {
			if (Item) {
				G_Device.destroyImageView(Item, nullptr, G_DLD);
				Item = nullptr;
			}
			G_SwapchainImageViews.clear();
		}

		G_SwapchainImages.clear();

		if (G_Swapchain) {
			G_Device.destroySwapchainKHR(G_Swapchain, nullptr, G_DLD);
			G_Swapchain = nullptr;
		}
	}
}

void RecreateSwapchain()
{
	DestroySwapchain();
	CreateSwapchain();
}

void ShutdownSwapchain()
{
	DestroySwapchain();

	if (G_OffscreenDescriptorPool) {
		G_Device.destroyDescriptorPool(G_OffscreenDescriptorPool, nullptr, G_DLD);
	}
	if (G_OffscreenDescriptorSetLayout) {
		G_Device.destroyDescriptorSetLayout(G_OffscreenDescriptorSetLayout, nullptr, G_DLD);
	}
}

void PerformInitialOwnershipTransfers()
{
	const std::size_t NumImages = G_SwapchainImages.size();

	if (G_GraphicsQueueFamilyIndex.value() != G_ComputeQueueFamilyIndex.value())
	{
		auto ComputeCommandBuffer = BeginSingleUseComputeCommandBuffer();
		for (std::uint32_t i = 0; i < NumImages; i++) {
			const vk::ImageMemoryBarrier ImageMemoryBarriers[] = {
				vk::ImageMemoryBarrier(
					vk::AccessFlagBits::eNone,
					vk::AccessFlagBits::eNone,
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eColorAttachmentOptimal,
					G_ComputeQueueFamilyIndex.value(),
					G_GraphicsQueueFamilyIndex.value(),
					std::get<G_ImageTuple_Image>(G_OffscreenColorRenderTargets[i]),
					vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),

				vk::ImageMemoryBarrier(
					vk::AccessFlagBits::eNone,
					vk::AccessFlagBits::eNone,
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eColorAttachmentOptimal,
					G_ComputeQueueFamilyIndex.value(),
					G_GraphicsQueueFamilyIndex.value(),
					std::get<G_ImageTuple_Image>(G_OffscreenCustomStencilRenderTargets[i]),
					vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),

			};
			ComputeCommandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlagBits::eAllCommands,
				{}, {}, {},
				ImageMemoryBarriers,
				G_DLD
				);
		}
		EndSingleUseComputeCommandBuffer(ComputeCommandBuffer);

		auto GraphicsCommandBuffer = BeginSingleUseGraphicsCommandBuffer();
		for (std::uint32_t i = 0; i < NumImages; i++) {
			const vk::ImageMemoryBarrier ImageMemoryBarriers[] = {
				vk::ImageMemoryBarrier(
					vk::AccessFlagBits::eNone,
					vk::AccessFlagBits::eNone,
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eGeneral,
					G_GraphicsQueueFamilyIndex.value(),
					G_ComputeQueueFamilyIndex.value(),
					std::get<G_ImageTuple_Image>(G_ColorRenderTargets[i]),
					vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)),
			};
			GraphicsCommandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlagBits::eAllCommands,
				{}, {}, {},
				ImageMemoryBarriers,
				G_DLD
				);

		}
		EndSingleUseGraphicsCommandBuffer(GraphicsCommandBuffer);

	}
}

void InitImGui()
{
	static constexpr vk::DescriptorPoolSize PoolSizes[1] = {
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 10),
	};
	static constexpr vk::DescriptorPoolCreateInfo DescriptorPoolCI = vk::DescriptorPoolCreateInfo(
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		10,
		1,
		PoolSizes
		);
	G_ImguiDescriptorPool = G_Device.createDescriptorPool(DescriptorPoolCI, nullptr, G_DLD);


	IMGUI_CHECKVERSION();
	G_ImGuiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(G_ImGuiContext);
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	//G_ConsolasFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesDefault());

	ImGui_ImplWin32_Init(G_Hwnd);

	auto VkLoaderFunction = [](const char* function_name, void* user_data) -> PFN_vkVoidFunction {
		auto pfn = G_DLD.vkGetInstanceProcAddr(G_VkInstance, function_name);
		if (!pfn) pfn = G_DLD.vkGetDeviceProcAddr(G_Device, function_name);
		return pfn;
	};
	ImGui_ImplVulkan_LoadFunctions(VkLoaderFunction, nullptr);

	auto VkCheckError = [](VkResult err){};

	ImGui_ImplVulkan_InitInfo VkInitInfo;
	VkInitInfo.Instance            = G_VkInstance;
	VkInitInfo.PhysicalDevice      = G_PhysicalDevice;
	VkInitInfo.Device              = G_Device;
	VkInitInfo.QueueFamily         = G_GraphicsQueueFamilyIndex.value();
	VkInitInfo.Queue               = G_GraphicsQueue;
	VkInitInfo.DescriptorPool      = G_ImguiDescriptorPool;
	VkInitInfo.RenderPass          = G_RenderPass;
	VkInitInfo.MinImageCount       = 2;
	VkInitInfo.ImageCount          = 2;
	VkInitInfo.MSAASamples         = static_cast<VkSampleCountFlagBits>(G_SampleCount);
	VkInitInfo.PipelineCache       = nullptr;
	VkInitInfo.Subpass             = 0;
	VkInitInfo.UseDynamicRendering = false;
	VkInitInfo.Allocator           = nullptr;
	VkInitInfo.CheckVkResultFn     = VkCheckError;
	VkInitInfo.MinAllocationSize   = 1024 * 1024;
	ImGui_ImplVulkan_Init(&VkInitInfo);

	ImGui::SetCurrentContext(nullptr);
}

void ShutdownImGui()
{
	ImGui::SetCurrentContext(G_ImGuiContext);

	ImGui_ImplVulkan_Shutdown();

	if (G_ImguiDescriptorPool) {
		G_Device.destroyDescriptorPool(G_ImguiDescriptorPool, nullptr, G_DLD);
		G_ImguiDescriptorPool = nullptr;
	}

	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext(G_ImGuiContext);
	ImGui::SetCurrentContext(nullptr);
	G_ImGuiContext = nullptr;
}

void InitGraphicsPipeline()
{

	static constexpr vk::PipelineLayoutCreateInfo PipelineLayoutCI = vk::PipelineLayoutCreateInfo{{}, 0, nullptr, 0, nullptr};
	G_GraphicsPipelineLayout = G_Device.createPipelineLayout(PipelineLayoutCI, nullptr, G_DLD);

	const vk::ShaderModule ShaderModuleVS = CreateShader("DefaultVS.spv");
	const vk::ShaderModule ShaderModuleFS = CreateShader("DefaultFS.spv");

	const std::array<vk::PipelineShaderStageCreateInfo, 2> ShaderStageCIs = {
		vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, ShaderModuleVS, "main"),
		vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, ShaderModuleFS, "main"),
		};

	static constexpr vk::PipelineVertexInputStateCreateInfo VertexInputStateCI = vk::PipelineVertexInputStateCreateInfo({}, 0, nullptr, 0, nullptr);

	static constexpr vk::PipelineInputAssemblyStateCreateInfo InputAssemblyCI = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList);
	static constexpr vk::PipelineTessellationStateCreateInfo TesselationStateCI = vk::PipelineTessellationStateCreateInfo();

	const vk::Viewport Viewport(0.0f, 0.0f, float(G_SwapchainExtent.width), float(G_SwapchainExtent.height));
	const vk::Rect2D Scissor(vk::Offset2D(), vk::Extent2D(G_SwapchainExtent.width, G_SwapchainExtent.height));
	static const vk::PipelineViewportStateCreateInfo ViewportStateCI = vk::PipelineViewportStateCreateInfo({}, 1, &Viewport, 1, &Scissor);

	static constexpr vk::PipelineRasterizationStateCreateInfo RasterizationStateCI = vk::PipelineRasterizationStateCreateInfo({}, {}, {}, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, {}, {}, {}, {}, 1.0f);
	const vk::PipelineMultisampleStateCreateInfo MultisampleCI = vk::PipelineMultisampleStateCreateInfo({}, G_SampleCount);

	static constexpr vk::PipelineDepthStencilStateCreateInfo DepthStencilCI = vk::PipelineDepthStencilStateCreateInfo({}, vk::True, vk::True, vk::CompareOp::eLess, vk::False, vk::False, {}, {}, 0.0f, 1.0f);
	static constexpr vk::PipelineColorBlendAttachmentState ColorBlendAttachmentState[2] =
		{
			vk::PipelineColorBlendAttachmentState({}, {}, {}, {}, {}, {}, {}, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA),
			vk::PipelineColorBlendAttachmentState({}, {}, {}, {}, {}, {}, {}, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
		};
	static constexpr vk::PipelineColorBlendStateCreateInfo BlendStateCI = vk::PipelineColorBlendStateCreateInfo({}, {}, {}, 2, ColorBlendAttachmentState);

	static constexpr std::array<vk::DynamicState, 2> DynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor};
	static constexpr vk::PipelineDynamicStateCreateInfo DynamicStateCI = vk::PipelineDynamicStateCreateInfo({}, static_cast<std::uint32_t>(DynamicStates.size()), DynamicStates.data());

	const vk::GraphicsPipelineCreateInfo GraphicsPipelineCI = vk::GraphicsPipelineCreateInfo(
		vk::PipelineCreateFlags(),
		ShaderStageCIs,
		&VertexInputStateCI,
		&InputAssemblyCI,
		&TesselationStateCI,
		&ViewportStateCI,
		&RasterizationStateCI,
		&MultisampleCI,
		&DepthStencilCI,
		&BlendStateCI,
		&DynamicStateCI,
		G_GraphicsPipelineLayout,
		G_RenderPass,
		0
		);

	G_GraphicsPipeline = G_Device.createGraphicsPipeline({}, GraphicsPipelineCI, nullptr, G_DLD).value;

	G_Device.destroyShaderModule(ShaderModuleVS, nullptr, G_DLD);
	G_Device.destroyShaderModule(ShaderModuleFS, nullptr, G_DLD);
}

void ShutdownGraphicsPipeline()
{
	if (G_GraphicsPipeline) {
		G_Device.destroyPipeline(G_GraphicsPipeline, nullptr, G_DLD);
		G_GraphicsPipeline = nullptr;
	}
	if (G_GraphicsPipelineLayout) {
		G_Device.destroyPipelineLayout(G_GraphicsPipelineLayout, nullptr, G_DLD);
		G_GraphicsPipelineLayout = nullptr;
	}
}

void InitComputePipeline()
{
	static constexpr vk::PipelineLayoutCreateInfo PipelineLayoutCI = vk::PipelineLayoutCreateInfo{{}, 1, &G_OffscreenDescriptorSetLayout, 0, nullptr};
	G_ComputePipelineLayout = G_Device.createPipelineLayout(PipelineLayoutCI, nullptr, G_DLD);

	const vk::ShaderModule ShaderModuleCS = CreateShader(G_SampleCount == vk::SampleCountFlagBits::e1 ? "OutlineCS.spv" : "OutlineMSCS.spv");
	const vk::PipelineShaderStageCreateInfo ShaderStageCI = vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eCompute, ShaderModuleCS, "main");

	const vk::ComputePipelineCreateInfo ComputePipelineCI = vk::ComputePipelineCreateInfo(
		{},
		ShaderStageCI,
		G_ComputePipelineLayout,
		{},
		{}
		);

	G_ComputePipeline = G_Device.createComputePipeline({}, ComputePipelineCI, nullptr, G_DLD).value;
	G_Device.destroyShaderModule(ShaderModuleCS, nullptr, G_DLD);
}

void ShutdownComputePipeline()
{
	if (G_ComputePipeline) {
		G_Device.destroyPipeline(G_ComputePipeline, nullptr, G_DLD);
		G_ComputePipeline = nullptr;
	}
	if (G_ComputePipelineLayout) {
		G_Device.destroyPipelineLayout(G_ComputePipelineLayout, nullptr, G_DLD);
		G_ComputePipelineLayout = nullptr;
	}
}

void InitModel()
{

}

void ShutdownModel()
{

}

