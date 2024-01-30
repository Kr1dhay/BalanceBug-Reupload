import { BrowserRouter, Routes, Route } from "react-router-dom";
import { HomePage } from "./pages/homepage";

function App() {
  return (
    <BrowserRouter>
      <div className="bg-[#F0E9FF] w-screen min-h-screen flex flex-col gap-5 items-center overflow-hidden">
        <Routes>
          <Route path="/" element={<HomePage />}></Route>
        </Routes>
      </div>
    </BrowserRouter>
  );
}

export default App;
