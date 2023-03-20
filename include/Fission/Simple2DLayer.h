#pragma once
#include <Fission/Core/Application.hh>
#include <Fission/Core/Engine.hh>

namespace Fission
{

  class Simple2DLayer : public Layer
  {
  public:
    virtual void OnCreate(Application * app) override
    {
      m_pRenderer2D = app->f_pEngine->GetRenderer<Renderer2D>( "$internal2D" );
    }

    Renderer2D * m_pRenderer2D;
  };

} // namespace Fission

