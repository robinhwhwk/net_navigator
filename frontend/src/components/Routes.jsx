import React, { useEffect, useState } from "react";
import {
    Box,
    Button,
    Flex,
    Input,
    InputGroup,
    Modal,
    ModalBody,
    ModalCloseButton,
    ModalContent,
    ModalFooter,
    ModalHeader,
    ModalOverlay,
    Stack,
    Text,
    useDisclosure
} from "@chakra-ui/react";

const RoutesContext = React.createContext({
    routes: [], fetchRoutes: () => {}
  })

export default function Routes() {
  const [routes, setRoutes] = useState([])
//   const config = {
//         method: 'POST',
//         headers: {
//             'Accept': 'application/json',
//             'Content-Type': 'application/json',
//         },
//         body: JSON.stringify(data)
//     }
  const fetchRoutes = async () => {
    const response = await fetch("http://localhost:8000/routes")
    const routes = await response.json()
    setRoutes(routes.data)
  }

  useEffect(() => {
    fetchRoutes()
  }, [])
  
  return (
    <RoutesContext.Provider value={{routes, fetchRoutes}}>
      <Stack spacing={5}>
        {routes.map((route) => (
          <b>{route.item}</b>
        ))}
      </Stack>
    </RoutesContext.Provider>
  )
}