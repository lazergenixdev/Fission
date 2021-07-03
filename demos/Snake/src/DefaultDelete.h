#pragma once

template <typename T>
struct DefaultDelete : public T { virtual void Destroy() override { delete this; } };
