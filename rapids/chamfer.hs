{-# LANGUAGE CPP, TemplateHaskell #-}
import Rapids

-- also in rubik.hs
cutBoth :: (Solid -> Solid) -> (Solid -> Solid) -> E V3 -> Solid
cutBoth a b ax = a (translate ax (-1) centeredCube) + b (translate ax 1 centeredCube)

model = centeredCube
  - cutBoth $white $yellow ez
  - cutBoth $red $green ex
  - cutBoth $blue $purple ey
  & chamfer 0.2

main = writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step") model
