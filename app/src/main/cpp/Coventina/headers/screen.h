#pragma once

#include <vector>

namespace screen{

  extern float bounds[2];

  extern int si; // index to bounds that represents which is smaller

  extern int pixelBounds[2];

  void init();

}

struct Screen {  //like a view in the java sdk. lol f java

  static std::vector<Screen*> screens;

  Screen();

  virtual void genGraphics();

  virtual void update(float dt);

};
