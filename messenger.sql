SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

CREATE DATABASE IF NOT EXISTS `messenger` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;
USE `messenger`;

DROP TABLE IF EXISTS `cadastro`;
CREATE TABLE `cadastro` (
  `id` int(11) NOT NULL,
  `usuario` varchar(15) NOT NULL,
  `senha` varchar(30) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `dados_conexao`;
CREATE TABLE `dados_conexao` (
  `id` int(11) NOT NULL,
  `id_usuario` int(11) NOT NULL,
  `ip` int(11) NOT NULL,
  `porta` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `mensagens_pendentes`;
CREATE TABLE `mensagens_pendentes` (
  `id` int(11) NOT NULL,
  `id_remetente` int(11) NOT NULL,
  `id_destinatario` int(11) NOT NULL,
  `mensagem` varchar(255) NOT NULL,
  `horario_envio` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `status`;
CREATE TABLE `status` (
  `id` int(11) NOT NULL,
  `id_usuario` int(11) NOT NULL,
  `last_seen` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


ALTER TABLE `cadastro`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `dados_conexao`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `mensagens_pendentes`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `status`
  ADD PRIMARY KEY (`id`);


ALTER TABLE `cadastro`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `dados_conexao`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `mensagens_pendentes`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `status`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
