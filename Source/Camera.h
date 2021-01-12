#pragma once

#include "Event.h"
#include "Common.h"

#include <algorithm>

namespace Graphics
{
	class Camera1;

	class CameraController
	{
	public:

		CameraController(Camera1 * pCamera) : m_camera(pCamera)
		{
		}

		void		Update(double milliSeconds);

	public: _RECV_EVENT_DECL1(CameraController, OnMouseMove);
	public: _RECV_EVENT_DECL1(CameraController, OnKeyDown);
	public: _RECV_EVENT_DECL1(CameraController, OnKeyUp);

	private:

		Camera1 *	m_camera;
		bool		m_init = true;
		int		m_pixelX = 0;
		int		m_pixelY = 0;

		// look at
		float		m_hAngle = 0.0f;
		float		m_vAngle = 0.0f;

		// move
		const float	m_speed = 10.0f;
		float		m_forwardFactor = 0.0f;
		float		m_rightFactor = 0.0f;
		float		m_upFactor = 0.0f;
		float		m_vFactor = 0.0f;
	};

	class Camera1
	{
	public:
		Camera1(float zNear, float zFar, float fov, float aspectRatio, const Vec3 pos)
			: m_zNear(zNear)
			, m_zFar(zFar)
			, m_fov(fov)
			, m_aspectRatio(aspectRatio)
			, m_up { 0.0f, 1.0f, 0.0f }
			, m_right { 1.0f, 0.0f, 0.0f }
			, m_forward { 0.0f, 0.0f, 1.0f }
			, m_hradian(0.0f)
			, m_vradian(0.0f)
			, m_pos(pos)
			, m_dirtyBits(BIT_ALL)
			, m_dir { 0.0f, 0.0f, 0.0f }
			, m_viewMatrix {}
			, m_projMatrix {}
			, m_controller(this)
		{
		}

		Camera1(const Camera1 &) = delete;
		Camera1(Camera1 && other) = delete;
		Camera1 & operator = (const Camera1 &) = delete;
		Camera1 & operator = (Camera1 && other) = delete;

		// Operations

		void			Move(const Vec3 & delta);

		// Properties

		void			SetHorizontalAngle(float fAngle); // Clockwise: 0.0f ~ 360.0f
		void			SetVerticalAngle(float fAngle); // Down: -90.0f, Up: 90.0f
		void			SetAspectRatio(float fAspectRatio);

		const Vec3 &		GetUp()
		{
			return m_up;
		}
		const Vec3 &		GetPos()
		{
			return m_pos;
		}
		const Vec3		GetDirection();
		const Matrix4x4 &	GetViewMatrix();
		const Matrix4x4 &	GetProjMatrix();

		CameraController &	GetController()
		{
			return m_controller;
		}

		// Events

	public: _SEND_EVENT(OnCameraDirChange);
	public: _SEND_EVENT(OnCameraPosChange);
	public: _RECV_EVENT_DECL1(Camera1, OnAspectRatioChange);

	private:

		// --------------------------------------------------------------------------
		// Camera lens parameters
		// --------------------------------------------------------------------------
		const float		m_zNear;
		const float		m_zFar;
		float			m_fov;
		float			m_aspectRatio;

		// --------------------------------------------------------------------------
		// World parameters
		// --------------------------------------------------------------------------
		const Vec3		m_up;
		const Vec3		m_right;
		const Vec3		m_forward;
		float			m_hradian; // horizontal rotation
		float			m_vradian; // vertical rotation

		Vec3			m_pos;

		// --------------------------------------------------------------------------
		// Cache
		// --------------------------------------------------------------------------
		enum
		{
			BIT_DIRECTION = 1,
			BIT_VIEW_MATRIX = 2,
			BIT_PROJ_MATRIX = 4,
			BIT_ALL = 7,
		};
		int			m_dirtyBits;
		Vec3			m_dir;
		Matrix4x4		m_viewMatrix;
		Matrix4x4		m_projMatrix;

		// --------------------------------------------------------------------------
		// Controllers
		// --------------------------------------------------------------------------
		CameraController	m_controller;
	};
}