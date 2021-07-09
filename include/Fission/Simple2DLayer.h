#include <Fission/Core/Scene.hh>

namespace Fission
{

  class Simple2DLayer : public IFLayer
  {
  public:
    virtual void OnCreate(FApplication * app) override
    {
      m_pRenderer2D = static_cast<IFRenderer2D *>( app->pEngine->GetRenderer("$internal2D") );
    }

    IFRenderer2D * m_pRenderer2D;
  };

} // namespace Fission

