#include <Fission/core/engine.hpp>

__FISSION_BEGIN__

auto render_main(void*) noexcept -> os::Thread_Result
{
    engine.setup();

    while (engine.render_frame());

    engine.shutdown();
    return {};
}

__FISSION_END__

#ifdef _os_main
_os_main()
{
    if (engine.create(on_create()))
        return engine.exit_code;

    engine.run();
    engine.destroy();

	return engine.exit_code;
}
#endif
