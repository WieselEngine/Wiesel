//   Copyright 2023 Metehan Gezer
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0

#include "w_demo.h"
#include "util/w_keycodes.h"
#include "util/w_math.h"
#include "scene/w_componentutil.h"
#include "layer/w_layerimgui.h"
#include "w_engine.h"

using namespace Wiesel;

namespace WieselDemo {

	DemoLayer::DemoLayer(DemoApplication& app) : m_App(app), Layer("Demo Layer") {
		m_Scene = app.GetScene();
		m_Renderer = Engine::GetRenderer();

		m_InputX = 0.0f;
		m_InputY = 0.0f;
		m_MouseSpeed = 0.001f;
		m_CameraMoveSpeed = 8.0f;
		m_LookXLimit = PI / 2.0f - (PI / 16.0f);
	}

	DemoLayer::~DemoLayer() = default;

	void DemoLayer::OnAttach() {
		LOG_DEBUG("OnAttach");

		// Loading a model to the scene
		{
			Entity entity = m_Scene->CreateEntity("Sponza");
			auto& model = entity.AddComponent<ModelComponent>();
			auto& transform = entity.GetComponent<TransformComponent>();
			transform.Scale = {0.01f, 0.01f, 0.01f};
			Engine::LoadModel(transform, model, "assets/models/sponza/sponza.gltf");
		}

		// Custom camera
		Reference<Camera> camera = CreateReference<Camera>(glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), m_Renderer->GetAspectRatio(), 60, 0.1f, 10000.0f);
		m_Renderer->AddCamera(camera);
		m_Renderer->SetActiveCamera(camera->GetId());
		m_Renderer->SetClearColor(0.02f, 0.02f, 0.04f);
		m_Renderer->SetVsync(false);
	}

	void DemoLayer::OnDetach() {
		LOG_DEBUG("OnDetach");
	}

	void DemoLayer::OnUpdate(float_t deltaTime) {
	//	LogInfo("OnUpdate " + std::to_string(deltaTime));
		const Reference<Camera>& camera = m_Renderer->GetActiveCamera();
		if (m_KeyManager.IsPressed(KeyW)) {
			camera->Move(camera->GetForward() * deltaTime * m_CameraMoveSpeed);
		} else if (m_KeyManager.IsPressed(KeyS)) {
			camera->Move(camera->GetBackward() * deltaTime * m_CameraMoveSpeed);
		}

		if (m_KeyManager.IsPressed(KeyA)) {
			camera->Move(camera->GetLeft() * deltaTime * m_CameraMoveSpeed);
		} else if (m_KeyManager.IsPressed(KeyD)) {
			camera->Move(camera->GetRight() * deltaTime * m_CameraMoveSpeed);
		}

		m_InputY = std::clamp(m_InputY, -m_LookXLimit, m_LookXLimit);
		camera->SetRotation(m_InputY, m_InputX, 0.0f);
	}

	void DemoLayer::OnEvent(Event& event) {
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<KeyPressedEvent>(WIESEL_BIND_EVENT_FUNCTION(OnKeyPress));
		dispatcher.Dispatch<KeyReleasedEvent>(WIESEL_BIND_EVENT_FUNCTION(OnKeyReleased));
		dispatcher.Dispatch<MouseMovedEvent>(WIESEL_BIND_EVENT_FUNCTION(OnMouseMoved));
	}

	bool DemoLayer::OnKeyPress(KeyPressedEvent& event) {
		if (event.GetKeyCode() == KeyF1) {
			m_App.Close();
			return true;
		} else if (event.GetKeyCode() == KeyEscape) {
			if (m_App.GetWindow()->GetCursorMode() == CursorModeRelative) {
				m_App.GetWindow()->SetCursorMode(CursorModeNormal);
			} else {
				m_App.GetWindow()->SetCursorMode(CursorModeRelative);
			}
			return true;
		}
		m_KeyManager.Set(event.GetKeyCode(), true);
		return true;
	}

	bool DemoLayer::OnKeyReleased(KeyReleasedEvent& event) {
		m_KeyManager.Set(event.GetKeyCode(), false);
		return true;
	}

	bool DemoLayer::OnMouseMoved(MouseMovedEvent& event) {
		if (m_App.GetWindow()->GetCursorMode() != CursorModeRelative) {
			return true;
		}

		m_InputX += m_MouseSpeed * ((m_App.GetWindowSize().Width / 2.0f) - event.GetX());
		m_InputY += m_MouseSpeed * ((m_App.GetWindowSize().Height / 2.0f) - event.GetY());
		return true;
	}

	DemoOverlay::DemoOverlay(DemoApplication& app, Reference<DemoLayer> demoLayer) : m_App(app), m_DemoLayer(demoLayer), Layer("Demo Overlay") {
	}

	DemoOverlay::~DemoOverlay() = default;

	void DemoOverlay::OnAttach() {
		LOG_DEBUG("OnAttach");

	}

	void DemoOverlay::OnDetach() {
		LOG_DEBUG("OnDetach");
	}

	void DemoOverlay::OnUpdate(float_t deltaTime) {

	}

	void DemoOverlay::OnEvent(Wiesel::Event& event) {
	}

	static entt::entity selectedEntity;
	static bool hasSelectedEntity = false;
	void DemoOverlay::OnImGuiRender() {
		static bool scenePropertiesOpen = true;
		//ImGui::ShowDemoWindow(&scenePropertiesOpen);
		if (ImGui::Begin("Scene Properties", &scenePropertiesOpen)) {
			auto& pos = Engine::GetRenderer()->GetActiveCamera()->GetPosition();
			ImGui::Text("Camera Pos");
			ImGui::LabelText("X", "%f", pos.x);
			ImGui::LabelText("Y", "%f", pos.y);
			ImGui::LabelText("Z", "%f", pos.z);

			bool changed = false;

			ImGui::SeparatorText("Controls");
			ImGui::InputFloat("Camera Speed", &m_DemoLayer->m_CameraMoveSpeed);
			if (ImGui::Checkbox("Wireframe Mode", Engine::GetRenderer()->IsWireframeEnabledPtr())) {
				Engine::GetRenderer()->SetRecreateGraphicsPipeline(true);
			}
			if (ImGui::Button("Recreate Pipeline")) {
				Engine::GetRenderer()->SetRecreateGraphicsPipeline(true);
			}
		}
		ImGui::End();

		static bool sceneOpen = true;
		if (ImGui::Begin("Scene Hierarchy", &sceneOpen)) {
			bool ignoreMenu = false;

			if (ImGui::IsMouseClicked(1, false))
				ImGui::OpenPopup("right_click_hierarcy");
			if (ImGui::BeginPopup("right_click_hierarcy")) {
				ignoreMenu = true;
				if (ImGui::BeginMenu("Add")) {
					if (ImGui::MenuItem("Empty Object")) {
						m_App.GetScene()->CreateEntity();
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndMenu();
				}
				ImGui::EndPopup();
			}

			for (const auto& item : m_App.GetScene()->GetAllEntitiesWith<TagComponent>()) {
				Entity entity = {item, &*m_App.GetScene()};
				auto& tagComponent = entity.GetComponent<TagComponent>();
				if (ImGui::Selectable(tagComponent.Tag.c_str(), hasSelectedEntity && selectedEntity == item, ImGuiSelectableFlags_None, ImVec2(0, 0))) {
					selectedEntity = item;
					hasSelectedEntity = true;
				}
				if (!ignoreMenu && ImGui::BeginPopupContextItem()) {
					selectedEntity = item;
					if (ImGui::Button("Remove Entity")) {
						m_App.GetScene()->DestroyEntity(entity);
						hasSelectedEntity = false;
					}
					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();

		static bool componentsOpen = true;
		if (ImGui::Begin("Components", &componentsOpen) && hasSelectedEntity) {
			Entity entity = {selectedEntity, &*m_App.GetScene()};
			TagComponent& tag = entity.GetComponent<TagComponent>();
			if (ImGui::InputText("##", &tag.Tag, ImGuiInputTextFlags_AutoSelectAll)) {
				bool hasSpace = false;
				for (int i = 0; i < tag.Tag.size(); i++) {
					const char& c = tag.Tag[i];
					if (c == ' ') {
						hasSpace = true;
					}
					break;
				}
				if (hasSpace) {
					TrimLeft(tag.Tag);
				}

				if (tag.Tag.empty()) {
					tag.Tag = "Entity";
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("add_component_popup");
			if (ImGui::BeginPopup("add_component_popup")) {
				GENERATE_COMPONENT_ADDERS(entity);
				/*if (ImGui::BeginMenu("Sub-menu")) {
					ImGui::MenuItem("Click me");
					ImGui::EndMenu();
				}*/
				//	ImGui::Separator();
				ImGui::EndPopup();
			}

			GENERATE_COMPONENT_EDITORS(entity);
		}
		ImGui::End();
		/*static int m_GizmoType = -1;
		m_GizmoType = ImGuizmo::OPERATION::ROTATE;
		if (hasSelectedEntity) {
			// Gizmos
			if (m_GizmoType != -1) {
				Entity entity = {selectedEntity, &*m_App.GetScene()};
				auto camera = Engine::GetRenderer()->GetActiveCamera();
				auto& size = Engine::GetRenderer()->GetWindowSize();

				// Editor camera
				glm::mat4 cameraProjection = camera->GetProjection();
				cameraProjection[1][1] *= -1;
				glm::mat4 cameraView = camera->GetViewMatrix();

				// Entity transform
				auto& tc = entity.GetComponent<TransformComponent>();
				glm::mat4 transform = tc.TransformMatrix;

				// Snapping
				bool snap = m_DemoLayer->m_KeyManager.IsPressed(KeyLeftControl);
				float snapValue = 0.5f; // Snap to 0.5m for translation/scale
				// Snap to 45 degrees for rotation
				if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
					snapValue = 45.0f;

				float snapValues[3] = { snapValue, snapValue, snapValue };

				ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
				ImGuizmo::Enable(true);
				ImGuizmo::DrawGrid(&cameraView[0][0], &cameraProjection[0][0], &transform[0][0], 200.0f);
				//ImGuizmo::SetRect(0, 0, size.Width, size.Height);
				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
									 (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
									 nullptr, snap ? snapValues : nullptr);

				if (ImGuizmo::IsUsing()) {
					glm::vec3 translation, rotation, scale;
					Math::DecomposeTransform(transform, translation, rotation, scale);

					glm::vec3 deltaRotation = rotation - tc.Rotation;
					tc.Position = translation;
					tc.Rotation += deltaRotation;
					tc.Scale = scale;
					tc.IsChanged = true;
				}
			}
		}*/
	}

	void DemoApplication::Init() {
		LOG_DEBUG("Init");
		Reference<DemoLayer> demoLayer = CreateReference<DemoLayer>(*this);
		PushLayer(demoLayer);
		PushOverlay(CreateReference<DemoOverlay>(*this, demoLayer));
	}

	DemoApplication::DemoApplication() {
		LOG_DEBUG("DemoApp constructor");
	}

	DemoApplication::~DemoApplication() {
		LOG_DEBUG("DemoApp destructor");
	}
}

// Called from entrypoint
Application* Wiesel::CreateApp() {
	return new WieselDemo::DemoApplication();
}
