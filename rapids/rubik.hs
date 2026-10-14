{-# LANGUAGE CPP, TemplateHaskell #-}
import Rapids

cutBoth :: (Solid -> Solid) -> (Solid -> Solid) -> E V3 -> Solid
cutBoth a b ax = a (translate ax (-1) centeredCube) + b (translate ax 1 centeredCube)

gaps :: Solid
gaps = $black $ translated ez (1/3) (translate ez (1/3 - 0.5) (scale 1 0.02 centeredCube))

solved :: Solid
solved = 
    centeredCube
      - cutBoth $white $yellow ez
      - cutBoth $red $green ex
      - cutBoth $blue $purple ey
      - rotatedDeg ex 90 gaps
      - rotateDeg ey 90 gaps

scrambled :: Solid
scrambled = l $ u $ r $ u' $ r $ u $ l solved

split :: E V3 -> Lens' Solid (Solid,Solid,Solid)
split (E ax) f a = f (a * translate (E ax) (1/3) centeredCube,
                      a * scale (E ax) (1/3) centeredCube,
                      a * translate (E ax) (-1/3) centeredCube)
  <&> \(a,b,c) -> a+b+c

d, u, r, l, f, b, d', u', r', l', f', b' :: Solid -> Solid
d  s = s & split ez . _1 %~ rotate ez (pi/2)
u  s = s & split ez . _3 %~ rotate ez (pi/2)
r  s = s & split ex . _3 %~ rotate ex (pi/2)
l  s = s & split ex . _1 %~ rotate ex (pi/2)
f  s = s & split ey . _3 %~ rotate ey (pi/2)
b  s = s & split ey . _1 %~ rotate ey (pi/2)
d' s = s & split ez . _1 %~ rotate ez (-pi/2)
u' s = s & split ez . _3 %~ rotate ez (-pi/2)
r' s = s & split ex . _3 %~ rotate ex (-pi/2)
l' s = s & split ex . _1 %~ rotate ex (-pi/2)
f' s = s & split ey . _3 %~ rotate ey (-pi/2)
b' s = s & split ey . _1 %~ rotate ey (-pi/2)

main = write $ scrambled + translate ex 2 solved

write :: Solid -> IO ()
write = writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step")
