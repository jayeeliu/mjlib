<?php
error_reporting(E_ALL ^ E_NOTICE);

$kv = new KVStore('172.16.139.60', 17879);
$table 	= 'tkv';
$key 	= 'key';
$value	= 'breuicnerofneromfoermfoermfoermfoemnroferofoerim';
echo "TEST: create -> get -> put -> get -> drop \n";
assert($kv->create($table) === true);
assert($kv->get($table, $key) === '');
assert($kv->put($table, $key, $value) === true);
assert($kv->get($table, $key) === $value);
assert($kv->drop($table) === true);


class KVStore {
	private $_fp = null;
	const IS_OK 	= 'OK',
		FLAG_LEN	= 128,	// 服务器端响应的标志位最大长度
		CMD_GET 	= "get %s %s\r\n",
		CMD_PUT		= "put %s %s %s\r\n",
		CMD_DEL 	= "del %s %s\r\n",
		CMD_CREATE 	= "create %s\r\n",
		CMD_DROP 	= "drop %s\r\n",
		CMD_QUIT 	= "quit\r\n";

	public function __construct($host, $port=17879) {
		$this->_fp = fsockopen($host, $port, $errno, $errmsg, 3);
		if ($errno) {
			throw new Exception($errno, $errno);
		}
	}

	/**
	 * get 返回格式为
	 * 标志位OK空格数据长度（字节数）\r\n，或错误号空格错误消息\r\n
	 * 如果OK，并且有数据返回，剩下的都是数据
	 */
	public function get($table, $key) {
		fwrite($this->_fp, sprintf(self::CMD_GET, $table, $key));
		$flag = fgets($this->_fp, self::FLAG_LEN);

		if (!$this->_is_ok($flag)) {
			return false;
		}
		list(, $len) = explode(' ', $flag);
		$len = (int)$len;
		return $len > 0 ? fread($this->_fp, $len) : '';
	}

	public function put($table, $key, $value) {
		fwrite($this->_fp, sprintf(self::CMD_PUT, $table, $key, $value));
		return self::_is_ok($this->_gets());
	}

	public function del($table, $key) {
		fwrite($this->_fp, sprintf(self::CMD_DEL, $table, $key));
		return self::_is_ok($this->_gets());
	}

	public function create($table) {
		fwrite($this->_fp, sprintf(self::CMD_CREATE, $table));
		return self::_is_ok($this->_gets());
	}

	public function drop($table) {
		fwrite($this->_fp, sprintf(self::CMD_DROP, $table));
		return self::_is_ok($this->_gets());
	}

	private function _gets() {
		if (!$this->_fp) {
			throw new Exception("connection off", 1);
		}
		return fgets($this->_fp, self::FLAG_LEN);
	}

	private function _is_ok($ret) {
		// var_dump($ret);
		return $ret && substr($ret, 0, 2) === self::IS_OK ? true : false;
	}

	public function __destruct() {
		if ($this->_fp) {
			fwrite($this->_fp, sprintf(self::QUIT));
			fclose($this->_fp);			
		}
	}
}