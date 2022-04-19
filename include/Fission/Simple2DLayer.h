#pragma once
#include <Fission/Core/Application.hh>

namespace Fission
{

  class Simple2DLayer : public IFLayer
  {
  public:
    virtual void OnCreate(FApplication * app) override
    {
      m_pRenderer2D = app->f_pEngine->GetRenderer<IFRenderer2D>( "$internal2D" );
    }

    IFRenderer2D * m_pRenderer2D;
  };

} // namespace Fission

