{-# LANGUAGE ForeignFunctionInterface #-}
{-# LANGUAGE ScopedTypeVariables #-}

-- Hello.hs
module Hello where

import Foreign

foreign export ccall helloFromHaskell :: IO ()
helloFromHaskell = putStrLn "Hello from Haskell"


foreign import ccall "dynamic" mkHello :: FunPtr (IO ()) -> IO ()

foreign export ccall registerHelloFromC :: FunPtr (IO ()) -> IO ()
registerHelloFromC :: FunPtr (IO ()) -> IO ()
registerHelloFromC func = do
  -- cast/convert our function pointer into a proper haskell function
  let hello = mkHello func
  -- normally we would use this moment to store it into some global storage for later use
  -- but today we'll just invoke the function right away to demonstrate it works
  hello