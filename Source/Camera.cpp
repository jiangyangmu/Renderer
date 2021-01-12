#include "Camera.h"
#include "Common.h"

#include <algorithm>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace Graphics
{

	void Camera1::Move(const Vec3 & delta)
	{
		m_pos.x += delta.x;
		m_pos.y += delta.y;
		m_pos.z += delta.z;
		m_dirtyBits |= BIT_VIEW_MATRIX;
		_DISPATCH_EVENT1(OnCameraPosChange, *this, GetPos());
	}

	void Camera1::SetHorizontalAngle(float fAngle)
	{
		float hradian = DegreeToRadian(fAngle);

		if ( m_hradian != hradian )
		{
			m_hradian = hradian;
			m_dirtyBits |= BIT_DIRECTION | BIT_VIEW_MATRIX;
			_DISPATCH_EVENT1(OnCameraDirChange, *this, GetDirection());
		}
	}

	void Camera1::SetVerticalAngle(float fAngle)
	{
		float vradian = DegreeToRadian(Bound(-90.0f, fAngle, 90.0f));

		if ( m_vradian != vradian )
		{
			m_vradian = vradian;
			m_dirtyBits |= BIT_DIRECTION | BIT_VIEW_MATRIX;
			_DISPATCH_EVENT1(OnCameraDirChange, *this, GetDirection());
		}
	}

	void Camera1::SetAspectRatio(float fAspectRatio)
	{
		m_aspectRatio = fAspectRatio;
		m_dirtyBits |= BIT_PROJ_MATRIX;
	}

	const Vec3 Camera1::GetDirection()
	{
		if (m_dirtyBits & BIT_DIRECTION)
		{
			m_dirtyBits &= ~BIT_DIRECTION;

			m_dir = Vec3::Transform(m_forward,
						Matrix4x4::RotationAxisLH(m_right, -m_vradian) * Matrix4x4::RotationAxisLH(m_up, m_hradian));
		}
		return m_dir;
	}

	const Matrix4x4 & Camera1::GetViewMatrix()
	{
		if (m_dirtyBits & BIT_VIEW_MATRIX)
		{
			m_dirtyBits &= ~BIT_VIEW_MATRIX;

			m_viewMatrix =
				Matrix4x4::Translation(-m_pos.x, -m_pos.y, -m_pos.z) *
				Matrix4x4::RotationAxisLH(m_up, -m_hradian) *
				Matrix4x4::RotationAxisLH(m_right, m_vradian);
		}
		return m_viewMatrix;
	}

	const Matrix4x4 & Camera1::GetProjMatrix()
	{
		if (m_dirtyBits & BIT_PROJ_MATRIX)
		{
			m_dirtyBits &= ~BIT_PROJ_MATRIX;

			m_projMatrix = Matrix4x4::PerspectiveFovLH(m_fov,
								   m_aspectRatio,
								   m_zNear,
								   m_zFar);
		}
		return m_projMatrix;
	}

	_RECV_EVENT_IMPL(Camera1, OnAspectRatioChange) ( void * sender, const float & aspectRatio )
	{
		UNREFERENCED_PARAMETER(sender);

		SetAspectRatio(aspectRatio);
	}


	void CameraController::Update(double milliSeconds)
	{
		// HACK: assume up is unit y
		ENSURE_TRUE(m_camera->GetUp().y == 1.0f);

		const Vec3 & up = m_camera->GetUp();

		Vec3 forwardDir, rightDir;

		forwardDir = m_camera->GetDirection();
		forwardDir.y = 0.0f;
		forwardDir = Vec3::Normalize(forwardDir);

		rightDir = Vec3::CrossLH(up, forwardDir);
		rightDir.y = 0.0f;
		rightDir = Vec3::Normalize(rightDir);

		float duration = static_cast< float >( milliSeconds / 1000.0f );

		Vec3 delta =
		{
			( m_forwardFactor * forwardDir.x + m_rightFactor * rightDir.x ) * duration * m_speed,
			( m_upFactor * up.y ) * duration * m_speed,
			( m_forwardFactor * forwardDir.z + m_rightFactor * rightDir.z ) * duration * m_speed,
		};

		if ( m_forwardFactor != 0.0f || m_rightFactor != 0.0f || m_upFactor != 0.0f )
		{
			m_camera->Move(delta);
		}
		if ( m_vFactor != 0.0f )
		{
			m_vAngle += 0.2f * milliSeconds * m_vFactor;
			m_camera->SetVerticalAngle(m_vAngle);
		}
	}

	_RECV_EVENT_IMPL(CameraController, OnMouseMove) ( void * sender, const win32::MouseEventArgs & args )
	{
		UNREFERENCED_PARAMETER(sender);

		if ( m_init )
		{
			m_init = false;
		}
		else
		{
			m_hAngle += 0.2f * ( args.pixelX - m_pixelX );
			m_vAngle -= 0.2f * ( args.pixelY - m_pixelY );
			m_vAngle = std::max(-90.0f, std::min(m_vAngle, 90.0f));
		}
		m_pixelX = args.pixelX;
		m_pixelY = args.pixelY;

		m_camera->SetHorizontalAngle(m_hAngle);
		m_camera->SetVerticalAngle(m_vAngle);
	}

	_RECV_EVENT_IMPL(CameraController, OnKeyDown) ( void * sender, const win32::KeyboardEventArgs & args )
	{
		UNREFERENCED_PARAMETER(sender);

		switch ( args.virtualKeyCode )
		{
			case 'W': m_forwardFactor = 1.0f; break;
			case 'S': m_forwardFactor = -1.0f; break;
			case 'A': m_rightFactor = -1.0f; break;
			case 'D': m_rightFactor = 1.0f; break;
			case 'Q': m_upFactor = -1.0f; break;
			case 'E': m_upFactor = 1.0f; break;
			case 'Z': m_vFactor = 1.0f; break;
			case 'C': m_vFactor = -1.0f; break;
			default: break;
		}
	}

	_RECV_EVENT_IMPL(CameraController, OnKeyUp) ( void * sender, const win32::KeyboardEventArgs & args )
	{
		UNREFERENCED_PARAMETER(sender);

		switch ( args.virtualKeyCode )
		{
			case 'W':
			case 'S': m_forwardFactor = 0.0f; break;
			case 'A':
			case 'D': m_rightFactor = 0.0f; break;
			case 'Q':
			case 'E': m_upFactor = 0.0f; break;
			case 'Z':
			case 'C': m_vFactor = 0.0f; break;
			default: break;
		}
	}
}